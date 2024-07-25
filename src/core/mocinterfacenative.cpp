/* ========================================================================
*    Copyright (C) 2013-2024 Blaze <blaze@vivaldi.net>
*
*    This file is part of eXo.
*
*    eXo is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    eXo is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with eXo.  If not, see <http://www.gnu.org/licenses/>.
* ======================================================================== */

#include "config.h"

#include <QVector>
#include <QStringList>
#include <QProcess>
#include <QRegularExpression>

#include <QTime>
#include <QDir>
#include "./socket/protocol.h"

#include "sysutils.h"
#include "mocinterfacenative.h"

#define OSD_OPT "OnSongChange=" INSTALL_PREFIX "/bin/moc-osd"
#define PLAYER_EXECUTABLE "mocp"

#define MOCN_ERROR -1
#define INT_SIZE sizeof(int)


MocInterfaceNative::MocInterfaceNative(QObject* parent) : PlayerInterface(parent),
    player(QStringLiteral(PLAYER_EXECUTABLE))
{
    if(SysUtils::findProcessId(player) < 0)  // check if player is running
        QProcess::startDetached(
                    player,
                    QStringList{QStringLiteral("-SO"),QStringLiteral(OSD_OPT)});
    startTimer(1000);

    connect(&socket, &QLocalSocket::disconnected,
            this, [this] { qWarning("disconecting!"); state = PState::Offline; });
}

void MocInterfaceNative::tryConnectToServer() {
    QString path(QDir::homePath() + "/.moc/socket2");
    socket.connectToServer(path);
    if (socket.waitForConnected()) {
        qDebug("Connected!");
        state = PState::Stop;
    }
}

void MocInterfaceNative::writeInt(int command) {
    // would not work on big endian platform
    const char data[] = {
        static_cast<char>(command & 0xFF),
        static_cast<char>((command >> 8) & 0xFF),
        static_cast<char>((command >> 16) & 0xFF),
        static_cast<char>((command >> 24) & 0xFF)
    };
    socket.write(QByteArray::fromRawData(data, sizeof(data)));
    socket.waitForBytesWritten();
}

int MocInterfaceNative::readInt() {
    QByteArray buf = socket.read(INT_SIZE);
    const char* data = buf.constData();
    // would not work on big endian platform
    int result = (
        static_cast<int>(static_cast<unsigned char>(data[0])) |
        (static_cast<int>(static_cast<unsigned char>(data[1])) << 8) |
        (static_cast<int>(static_cast<unsigned char>(data[2])) << 16)|
        (static_cast<int>(static_cast<unsigned char>(data[3])) << 24)
        );
    return result;
}

int MocInterfaceNative::readIntCommand() {
    if(socket.waitForReadyRead()) {
        int result = readInt();
        if(result == EV_DATA) {
            return readInt();
        }
    }
    return MOCN_ERROR;
}

QString MocInterfaceNative::readStringCommand() {
    int result = readIntCommand();
    if(result > 0) {
        qDebug() << "trying to read " << result << " bytes!";
        QByteArray buf = socket.read(result);
        return QString::fromLocal8Bit(buf);
    }
    return QString();
}

QVector<QString> MocInterfaceNative::readTagCommand() {
    int result = readIntCommand();
    QVector<QString> data;
    if(result > 0 && result < INT_MAX) {
        for(int i=0; i<3; i++) {
            qDebug() << "trying to read " << result << " bytes!";
            QByteArray buf = socket.read(result);
            data.append(QString::fromLocal8Bit(buf));
            result = readInt();
        }
    }
    socket.readAll();  //flush
    return data;
}

int MocInterfaceNative::sendCommand(int command) {
    writeInt(command);
    return readIntCommand();
}

QString MocInterfaceNative::sendStringCommand(int command) {
    writeInt(command);
    return readStringCommand();
}

QVector<QString> MocInterfaceNative::sendTagCommand(int command) {
    writeInt(command);
    return readTagCommand();
}

void MocInterfaceNative::runServer() {
    if(SysUtils::findProcessId(player) < 0) {  // check if player is running
        QProcess p;
        p.start(player,
                QStringList{ QStringLiteral("-SO"), QStringLiteral(OSD_OPT) },
                QIODevice::NotOpen);
        p.waitForFinished();
    }
}

PState MocInterfaceNative::updateInfo() {
    if(state == PState::Offline) {
        tryConnectToServer();
    }

    int receivedState = sendCommand(CMD_GET_STATE);
    if(receivedState == STATE_STOP)
        return PState::Stop;
    else if(receivedState == STATE_PLAY)
        state = PState::Play;
    else if(receivedState == STATE_PAUSE)
        state = PState::Pause;
    else
        qWarning() << "invalid state info";

    track.file = sendStringCommand(CMD_GET_SNAME);
    track.currSec = sendCommand(CMD_GET_CTIME);
    track.isStream = track.file.startsWith("http") || track.file.startsWith("ftp");

    QVector<QString> tagInfo = sendTagCommand(CMD_GET_TAGS);
    if(tagInfo.size() == 3) {
        track.artist = std::move(tagInfo.at(0));
        track.title = std::move(tagInfo.at(1));
        track.album = std::move(tagInfo.at(2));
    }

    track.caption = QString();
    track.totalSec = 10*60;
    track.totalTime = QTime().addSecs(track.totalSec).toString("mm::ss");

    if(track.isStream) {
        track.totalSec = 10*60;
        const QString dash = QStringLiteral(" - ");
        if(track.caption.contains(dash)) {
            track.artist = track.caption.section(dash, 0, 0);
            track.title = track.caption.section(dash, 1, -1);
        }
    }
    return state;
}

const QString MocInterfaceNative::id() const {
    return QStringLiteral("music on console");
}

#define SEND_COMMAND(__method, __option)\
    void MocInterfaceNative::__method() {\
        sendCommand(__option);\
    }

SEND_COMMAND(play, CMD_PLAY)
SEND_COMMAND(pause,CMD_PAUSE)
SEND_COMMAND(playPause,CMD_PAUSE)
SEND_COMMAND(prev, CMD_PREV)
SEND_COMMAND(next, CMD_NEXT)
SEND_COMMAND(stop, CMD_STOP)
SEND_COMMAND(quit, CMD_QUIT)
SEND_COMMAND(clearPlaylist, CMD_CLI_PLIST_CLEAR)

#define SEND_COMMAND_PARAM(__method, __option)\
    void MocInterfaceNative::__method(int param) {\
    QProcess::startDetached(player,\
    QStringList() << QString(QStringLiteral(__option)).arg(param));\
    }

SEND_COMMAND_PARAM(jump, "-j%1s")
SEND_COMMAND_PARAM(seek, "-k%1")
SEND_COMMAND_PARAM(volume, "-v%1")
SEND_COMMAND_PARAM(changeVolume, "-v+%1")

void MocInterfaceNative::showPlayer() {
    const QVector<QString> apps = SysUtils::findFullPaths(
                QVector<QString> {
                    QStringLiteral("x-terminal-emulator"),
                    QStringLiteral("gnome-terminal"),
                    QStringLiteral("konsole"),
                    QStringLiteral("xfce4-terminal"),
                    QStringLiteral("lxterminal") });
    const QString term = apps.isEmpty() ? QStringLiteral("xterm") : apps.at(0);
    QProcess::startDetached(
                term,
                QStringList{QStringLiteral("-e"),
                            QStringLiteral(PLAYER_EXECUTABLE " -O " OSD_OPT)});
}

void MocInterfaceNative::openUri(const QString& file) {
    runServer();
    QProcess::startDetached(player,
                            QStringList() << QStringLiteral("-l") << file);
}

void MocInterfaceNative::appendFile(const QStringList& files) {
    runServer();
    QProcess::startDetached(player,
                            QStringList() << QStringLiteral("-a") << files);
}

void MocInterfaceNative::timerEvent(QTimerEvent* event) {
    Q_UNUSED(event);
    notify();
}

void MocInterfaceNative::shutdown() {

}
