/* ========================================================================
*    Copyright (C) 2013-2023 Blaze <blaze@vivaldi.net>
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

#ifndef PLAYERINTERFACE_H
#define PLAYERINTERFACE_H

#include <QObject>

#define PLAYER PlayerInterface::self()


enum class PlayerState { Offline, Stop, Play, Pause };
struct PlayerTrack {
    bool isStream;
    int totalSec, currSec;
    QString artist, title, album, file, totalTime, caption;
};
using PState = PlayerState;
using PTrack = PlayerTrack;


class PlayerInterface : public QObject
{
    Q_OBJECT

    static PlayerInterface* object;
    QString getCover();

protected:
    PTrack track{};
    void notify();
    virtual PState updateInfo() = 0;

public:
    explicit PlayerInterface(QObject* parent = nullptr);
    static PlayerInterface* self() { return object; }
    const PTrack& getTrack() const { return track; }
    virtual const QString id() const = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void playPause() = 0;
    virtual void prev() = 0;
    virtual void next() = 0;
    virtual void stop() = 0;
    virtual void quit() = 0; // quit the player app
    virtual void jump(int position) = 0;
    virtual void seek(int offset) = 0;
    virtual void volume(int level) = 0;
    virtual void changeVolume(int delta) = 0;
    virtual void showPlayer() = 0;
    virtual void openUri(const QString& uri) = 0;
    virtual void appendFile(const QStringList& files) = 0;
    virtual void clearPlaylist() = 0;
    virtual void shutdown() = 0;
    Q_SIGNAL void newStatus(PState);
    Q_SIGNAL void newTrack(const QString&, bool check = false);
};

#endif // PLAYERINTERFACE_H
