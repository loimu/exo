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
constexpr int TAGS_TIME	= 0x02;


struct TagInfo
{
    bool success = false;
    int number = 0;
    int time  = -1;
    int filled = 0;
    QString artist;
    QString title;
    QString album;
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
    QString path(QDir::homePath() + QStringLiteral("/.moc/socket2"));
    socket.connectToServer(path);
    if(socket.waitForConnected(500)) {
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
    return (
        static_cast<int>(static_cast<unsigned char>(data[0])) |
        (static_cast<int>(static_cast<unsigned char>(data[1])) << 8) |
        (static_cast<int>(static_cast<unsigned char>(data[2])) << 16)|
        (static_cast<int>(static_cast<unsigned char>(data[3])) << 24)
        );  // would not work on big endian platform
}

void MocInterfaceNative::writeString(QLocalSocket& socket, const QString& string) {
    QByteArray data(string.toLocal8Bit());
    writeInt(socket, data.size());
    socket.write(data);
    socket.waitForBytesWritten();
}

QString MocInterfaceNative::readString(QLocalSocket& socket) {
    int length = readInt(socket);
    if(length > 0) {
        QByteArray buf = socket.read(length);
        return QString::fromLocal8Bit(buf);
    }
    return QString();
}

bool MocInterfaceNative::readPingResponse(QLocalSocket& socket) {
    int result = readInt(socket);
    if(result == EV_PONG) {
        return true;
    }
    return false;
}

int MocInterfaceNative::readIntResponse(QLocalSocket& socket) {
    int result = readInt(socket);
    if(result == EV_DATA) {
        return readInt(socket);
    }
    return MOCN_ERROR;
}

QString MocInterfaceNative::readStringResponse(QLocalSocket& socket) {
    int result = readInt(socket);
    if(result == EV_DATA) {
        return readString(socket);
    }
    return QString();
}

void MocInterfaceNative::readTagInfo(QLocalSocket& socket, TagInfo& tagInfo) {
    tagInfo.title = readString(socket);
    tagInfo.artist = readString(socket);
    tagInfo.album = readString(socket);
    tagInfo.number = readInt(socket);
    tagInfo.time = readInt(socket);
    tagInfo.filled = readInt(socket);
    tagInfo.success = true;
}

TagInfo MocInterfaceNative::readTagResponse(QLocalSocket& socket) {
    TagInfo data;
    int result = readInt(socket);
    if(result == EV_DATA) {
        readTagInfo(socket, data);
    }
    return data;
}

TagInfo MocInterfaceNative::readFileTagResponse(QLocalSocket& socket, const QString& file) {
    TagInfo data;
    int result = readInt(socket);
    if(result == EV_FILE_TAGS) {
        QString tagFile = readString(socket);
        if(tagFile == file) {
            readTagInfo(socket, data);
        }
    }
    return data;
}

bool MocInterfaceNative::sendPingCommand(QLocalSocket& socket) {
    writeInt(socket, CMD_PING);
    if(socket.waitForReadyRead()) {
        return readPingResponse(socket);
    }
    return false;
}

void MocInterfaceNative::sendCommand(QLocalSocket& socket, int command) {
    writeInt(socket, command);
}

void MocInterfaceNative::sendCommandParam(QLocalSocket& socket, int command, int param) {
    writeInt(socket, command);
    writeInt(socket, param);
}

void MocInterfaceNative::sendCommandParam(QLocalSocket& socket, int command, const QString& param) {
    writeInt(socket, command);
    writeString(socket, param);
}

int MocInterfaceNative::sendIntCommand(QLocalSocket& socket, int command) {
    writeInt(socket, command);
    if(socket.waitForReadyRead()) {
        return readIntResponse(socket);
    }
    return MOCN_ERROR;
}

QString MocInterfaceNative::sendStringCommand(QLocalSocket& socket, int command) {
    writeInt(socket, command);
    if(socket.waitForReadyRead()) {
        return readStringResponse(socket);
    }
    return QString();
}

TagInfo MocInterfaceNative::sendTagCommand(QLocalSocket& socket) {
    writeInt(socket, CMD_GET_TAGS);
    if(socket.waitForReadyRead()) {
        return readTagResponse(socket);
    }
    return TagInfo();
}

TagInfo MocInterfaceNative::sendFileTagCommand(QLocalSocket& socket, const QString& file) {
    writeInt(socket, CMD_GET_FILE_TAGS);
    writeString(socket, file);
    writeInt(socket, TAGS_TIME);
    if(socket.waitForReadyRead()) {
        return readFileTagResponse(socket, file);
    }
    return TagInfo();
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
        track.caption.clear();
        return state;
    }
    if(!sendPingCommand(socket)) {
        socket.disconnectFromServer();
        return state;
    }

    int receivedState = sendIntCommand(socket, CMD_GET_STATE);
    if(receivedState == STATE_STOP) {
        state = PState::Stop;
        track.caption.clear();
        return state;
    }
    else if(receivedState == STATE_PLAY)
        state = PState::Play;
    else if(receivedState == STATE_PAUSE)
        state = PState::Pause;
    else {
        socket.disconnectFromServer();
        return state;
    }

    const int ctime = sendIntCommand(socket, CMD_GET_CTIME);
    const QString file = sendStringCommand(socket, CMD_GET_SNAME);
    track.isStream = track.file.startsWith("http") || track.file.startsWith("ftp");
    const TagInfo tagInfo = track.isStream ? sendTagCommand(socket)
                                           : sendFileTagCommand(socket, file);
    socket.disconnectFromServer();

    int trackNumber = 0;
    if(!file.isEmpty()) {
        track.file = std::move(file);
    }
    if(ctime > 0) {
        track.currSec = ctime;
    }
    if(tagInfo.success && !tagInfo.title.isEmpty()) {
        track.title = std::move(tagInfo.title);
        track.artist = std::move(tagInfo.artist);
        track.album = std::move(tagInfo.album);
        trackNumber = tagInfo.number;
        int totalSec = tagInfo.filled & TAGS_TIME ? tagInfo.time : -1;
        track.totalSec = totalSec > 0 ? totalSec : 10*60;
    } else {
        return state;
    }

    track.isStream = track.file.startsWith("http") || track.file.startsWith("ftp");
    track.caption = track.isStream
                        ? track.title
                        : QString("%1 %2 - %3 (%4)").arg(
                              QString::number(trackNumber),
                              track.artist,
                              track.title,
                              track.album);
    track.totalTime = QTime(0, 0, 0).addSecs(track.totalSec).toString("mm:ss");

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
}

SEND_COMMAND(quit, CMD_QUIT)

#define SEND_COMMAND_PARAM(__method, __option)\
void MocInterfaceNative::__method(int param) {\
        QLocalSocket socket;\
        tryConnectToServer(socket);\
        sendCommandParam(socket, __option, param);\
        socket.disconnectFromServer();\
}

SEND_COMMAND_PARAM(jump, CMD_JUMP_TO)
SEND_COMMAND_PARAM(seek, CMD_SEEK)
SEND_COMMAND_PARAM(volume, CMD_SET_MIXER)

#define SEND_COMMAND_PARAM_EMPTY(__method, __option, __final)\
void MocInterfaceNative::__method() {\
        QLocalSocket socket;\
        tryConnectToServer(socket);\
        sendCommandParam(socket, __option, __final);\
        socket.disconnectFromServer();\
}

SEND_COMMAND_PARAM_EMPTY(stop, CMD_STOP, CMD_DISCONNECT)
SEND_COMMAND_PARAM_EMPTY(pause, CMD_PAUSE, CMD_DISCONNECT)
SEND_COMMAND_PARAM_EMPTY(prev, CMD_PREV, CMD_DISCONNECT)
SEND_COMMAND_PARAM_EMPTY(next, CMD_NEXT, CMD_DISCONNECT)

void MocInterfaceNative::playPause() {
    QLocalSocket socket;
    tryConnectToServer(socket);
    int result = sendIntCommand(socket, CMD_GET_STATE);
    if(result == STATE_PAUSE) {
        sendCommandParam(socket, CMD_UNPAUSE, CMD_DISCONNECT);
    }
    else if (result == STATE_PLAY) {
        sendCommandParam(socket, CMD_PAUSE, CMD_DISCONNECT);
    }
    socket.disconnectFromServer();
}

void MocInterfaceNative::changeVolume(int diff) {
    QLocalSocket socket;
    tryConnectToServer(socket);
    int result = sendIntCommand(socket, CMD_GET_MIXER);
    result += diff;
    qBound(0, result, 100);
    sendCommandParam(socket, CMD_SET_MIXER, result);
    socket.disconnectFromServer();
}

void MocInterfaceNative::showPlayer() {
    const QVector<QString> apps = SysUtils::findFullPaths(
        QVector<QString> {
            QStringLiteral("x-terminal-emulator"),
            QStringLiteral("gnome-terminal"),
            QStringLiteral("konsole"),
            QStringLiteral("xfce4-terminal"),
            QStringLiteral("lxterminal"),
            QStringLiteral("alacritty"),
            QStringLiteral("urxvt")
        });
    const QString term = apps.isEmpty() ? QStringLiteral("xterm") : apps.at(0);
    QProcess::startDetached(
        term,
        QStringList{QStringLiteral("-e"),
                    QStringLiteral(PLAYER_EXECUTABLE " -O " OSD_OPT)});
}

// native playlist operations is going to be the next big thing to implement here
void MocInterfaceNative::play() {
    runServer();
    QProcess::startDetached(player,
                            QStringList() << QStringLiteral("-p"));
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

void MocInterfaceNative::clearPlaylist() {
    QProcess::startDetached(player,
                            QStringList() << QStringLiteral("-c"));
}

void MocInterfaceNative::timerEvent(QTimerEvent* event) {
    Q_UNUSED(event);
    notify();
}

void MocInterfaceNative::shutdown() {
    // nothing to shutdown as far as we are not triggering QProcess periodically
}
