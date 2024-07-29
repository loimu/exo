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

#ifndef MOCINTERFACENATIVE_H
#define MOCINTERFACENATIVE_H

#include "playerinterface.h"

class QLocalSocket;
class TagInfo;


class MocInterfaceNative : public PlayerInterface
{
    const QString player;
    void runServer();
    PState updateInfo() override;
    void timerEvent(QTimerEvent* event) override;

    PState state = PState::Offline;

    bool tryConnectToServer(QLocalSocket& socket);
    void writeInt(QLocalSocket& socket, int command);
    int readInt(QLocalSocket& socket);
    void writeString(QLocalSocket& socket, const QString& string);
    QString readString(QLocalSocket& socket);
    bool readPingResponse(QLocalSocket& socket);
    int readIntResponse(QLocalSocket& socket);
    QString readStringResponse(QLocalSocket& socket);
    TagInfo readTagResponse(QLocalSocket& socket);
    TagInfo readFileTagResponse(QLocalSocket& socket, const QString& file);
    bool sendPingCommand(QLocalSocket& socket);
    void sendCommand(QLocalSocket& socket, int command);
    void sendCommandParam(QLocalSocket& socket, int command, int param);
    void sendCommandParam(QLocalSocket& socket, int command, const QString& param);
    int sendIntCommand(QLocalSocket& socket, int command);
    QString sendStringCommand(QLocalSocket& socket, int command);
    TagInfo sendTagCommand(QLocalSocket& socket);
    TagInfo sendFileTagCommand(QLocalSocket& socket, const QString& file);

public:
    explicit MocInterfaceNative(QObject* parent = nullptr);
    const QString id() const override;
    void play() override;
    void pause() override;
    void playPause() override;
    void prev() override;
    void next() override;
    void stop() override;
    void quit() override;
    void jump(int) override;
    void seek(int) override;
    void volume(int) override;
    void changeVolume(int) override;
    void showPlayer() override;
    void openUri(const QString&) override;
    void appendFile(const QStringList&) override;
    void clearPlaylist() override;
    void shutdown() override;
};

#endif // MOCINTERFACE_H
