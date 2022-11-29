/* ========================================================================
*    Copyright (C) 2013-2022 Blaze <blaze@vivaldi.net>
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

#ifndef MOCINTERFACE_H
#define MOCINTERFACE_H

#include "playerinterface.h"

class QProcess;

class MocInterface : public PlayerInterface
{
    QProcess* moc;
    const QString player;
    void runServer();
    PState updateInfo() override;
    void timerEvent(QTimerEvent* event) override;

public:
    explicit MocInterface(QObject* parent = nullptr);
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
