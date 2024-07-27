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
#include <QLocalSocket>
#include <QTime>
#include <QDir>

#include "socket/protocol.h"
#include "sysutils.h"
#include "mocinterfacenative.h"

#define OSD_OPT "OnSongChange=" INSTALL_PREFIX "/bin/moc-osd"
#define PLAYER_EXECUTABLE "mocp"

constexpr int MOCN_ERROR = -1;
constexpr int INT_SIZE = sizeof(int);

constexpr char TAG_END[] = {
    static_cast<char>(0xff),
    static_cast<char>(0xff),
    static_cast<char>(0xff)
};


MocInterfaceNative::MocInterfaceNative(QObject* parent) : PlayerInterface(parent),
    player(QStringLiteral(PLAYER_EXECUTABLE))
{
    if(SysUtils::findProcessId(player) < 0)  // check if player is running
        QProcess::startDetached(
            player,
            QStringList{QStringLiteral("-SO"),QStringLiteral(OSD_OPT)});
    startTimer(1000);
}

bool MocInterfaceNative::tryConnectToServer(QLocalSocket& socket) {
    QString path(QDir::homePath() + "/.moc/socket2");
    socket.connectToServer(path);
    if(socket.waitForConnected()) {
        return true;
    }
    return false;
}

void MocInterfaceNative::writeInt(QLocalSocket& socket, int command) {
    const char data[] = {
        static_cast<char>(command & 0xFF),
        static_cast<char>((command >> 8) & 0xFF),
        static_cast<char>((command >> 16) & 0xFF),
        static_cast<char>((command >> 24) & 0xFF)
    };  // would not work on big endian platform
    socket.write(QByteArray::fromRawData(data, sizeof(data)));
    socket.waitForBytesWritten();
}

int MocInterfaceNative::readInt(QLocalSocket& socket) {
    QByteArray buf = socket.read(INT_SIZE);
    const char* data = buf.constData();
    int result = (
        static_cast<int>(static_cast<unsigned char>(data[0])) |
        (static_cast<int>(static_cast<unsigned char>(data[1])) << 8) |
        (static_cast<int>(static_cast<unsigned char>(data[2])) << 16)|
        (static_cast<int>(static_cast<unsigned char>(data[3])) << 24)
        );  // would not work on big endian platform
    return result;
}

bool MocInterfaceNative::readPingResponse(QLocalSocket& socket) {
    if(socket.waitForReadyRead()) {
        int result = readInt(socket);
        if(result == EV_PONG) {
            return true;
        }
    }
    return false;
}

int MocInterfaceNative::readIntResponse(QLocalSocket& socket) {
    if(socket.waitForReadyRead()) {
        int result = readInt(socket);
        if(result == EV_DATA) {
            return readInt(socket);
        }
    }
    return MOCN_ERROR;
}

QString MocInterfaceNative::readStringResponse(QLocalSocket& socket) {
    int result = readIntResponse(socket);
    if(result > 0) {
        QByteArray buf = socket.read(result);
        return QString::fromLocal8Bit(buf);
    }
    return QString();
}

QVector<QString> MocInterfaceNative::readTagResponse(QLocalSocket& socket) {
    int result = readIntResponse(socket);
    QVector<QString> data;
    while(result > 0) {
        QByteArray buf = socket.read(result);
        if(buf == QByteArray(TAG_END)) {
            qDebug("Complete!");
            break;
        }
        data.append(QString::fromLocal8Bit(buf));
        result = readInt(socket);
    }
    return data;
}

bool MocInterfaceNative::sendPingCommand(QLocalSocket& socket) {
    writeInt(socket, CMD_PING);
    return readPingResponse(socket);
}

int MocInterfaceNative::sendCommand(QLocalSocket& socket, int command) {
    writeInt(socket, command);
    return readIntResponse(socket);
}

QString MocInterfaceNative::sendStringCommand(QLocalSocket& socket, int command) {
    writeInt(socket, command);
    return readStringResponse(socket);
}

QVector<QString> MocInterfaceNative::sendTagCommand(QLocalSocket& socket) {
    writeInt(socket, CMD_GET_TAGS);
    return readTagResponse(socket);
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
    QLocalSocket socket;
    if(!tryConnectToServer(socket)) {
        state = PState::Offline;
    }
    if(!sendPingCommand(socket)) {
        return state;
    }

    int receivedState = sendCommand(socket, CMD_GET_STATE);
    if(receivedState == STATE_STOP)
        return PState::Stop;
    else if(receivedState == STATE_PLAY)
        state = PState::Play;
    else if(receivedState == STATE_PAUSE)
        state = PState::Pause;
    else {
        return state;
    }

    const QString file = sendStringCommand(socket, CMD_GET_SNAME);
    const int ctime = sendCommand(socket, CMD_GET_CTIME);
    const QVector<QString> tagInfo = sendTagCommand(socket);

    socket.disconnectFromServer();

    if(!file.isEmpty()) {
        track.file = std::move(file);
    }
    track.isStream = track.file.startsWith("http") || track.file.startsWith("ftp");
    if(tagInfo.size() == 3) {
        track.title = std::move(tagInfo.at(0));
        track.artist = std::move(tagInfo.at(1));
        track.album = std::move(tagInfo.at(2));
    }
    if(ctime > 0) {
        track.currSec = ctime;
    }

    track.caption = track.isStream ? track.title
                                   : QString("%1 - %2").arg(track.artist, track.title);
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
        QLocalSocket socket;\
        tryConnectToServer(socket);\
        sendCommand(socket, __option);\
        socket.disconnectFromServer();\
        socket.waitForDisconnected();\
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
    // nothing to shutdown as far as we are not triggering QProcess periodically
}