/* ========================================================================
*    Copyright (C) 2013-2018 Blaze <blaze@vivaldi.net>
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

#include "config.h"

#include <QObject>

class PlayerInterface : public QObject
{
    Q_OBJECT

public:
    enum State { Offline, Stop, Play, Pause };

    struct Track {
        bool isStream;
        int totalSec, currSec;
        QString artist, title, album, file, totalTime, caption, cover;
    };

    explicit PlayerInterface(QObject* parent = nullptr);
    static PlayerInterface* self() { return object; }
    static Track* getTrack() { return ptrack; }
    virtual QString id() = 0;
    virtual bool play() = 0;
    virtual bool pause() = 0;
    virtual bool playPause() = 0;
    virtual bool prev() = 0;
    virtual bool next() = 0;
    virtual bool stop() = 0;
    virtual bool quit() = 0;
    virtual bool jump(int position) = 0;
    virtual bool seek(int offset) = 0;
    virtual bool volume(int level) = 0;
    virtual bool changeVolume(int delta) = 0;
    virtual void showPlayer() = 0;
    virtual bool openUri(const QString& uri) = 0;
    virtual bool appendFile(const QStringList& files) = 0;

protected:
    Track track;
    void timerEvent(QTimerEvent* event);
    virtual State getInfo() = 0;

private:
    static PlayerInterface* object;
    static Track* ptrack;
    void getCover();

signals:
    void newStatus(PlayerInterface::State);
    void newTrack();
#ifdef BUILD_LASTFM
    void trackListened(const QString&, const QString&, const QString&, int);
    void trackChanged(const QString&, const QString&, int);
#endif // BUILD_LASTFM
};

using PIState = PlayerInterface::State;
using PITrack = PlayerInterface::Track;

#endif // PLAYERINTERFACE_H
