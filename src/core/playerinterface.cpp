/* ========================================================================
*    Copyright (C) 2013-2017 Blaze <blaze@vivaldi.net>
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

#include <QTimerEvent>
#include <QDir>

#include "playerinterface.h"

PlayerInterface* PlayerInterface::object = nullptr;

PlayerInterface::PlayerInterface(QObject* parent) : QObject(parent), track()
{
    if(object)
        qFatal("PlayerInterface: only one instance is allowed");
    object = this;
}

void PlayerInterface::timerEvent(QTimerEvent *event) {
    State currentStatus = getInfo();
#ifdef BUILD_DBUS
    static QString title = QString();
    if(title != track.title) {
        title = track.title;
        emit newTrack();
    }
#endif // BUILD_DBUS
    static State status = Offline;
    if(status != currentStatus) {
        status = currentStatus;
        emit newStatus(status);
        if(status == Offline)
            emit updateStatus(tr("Player isn't running."),
                              QString(), QString(), QString());
        if(status == Stop)
            emit updateStatus(tr("Stopped"), QString(), QString(), QString());
        if(status == Pause)
            emit updateStatus(
                    track.title, track.currTime, track.totalTime, cover());
    }
    if(status == Play) {
        emit updateStatus(
                    track.title, track.currTime, track.totalTime, cover());
        scrobble();
    }
}

void PlayerInterface::scrobble() {
    static QString nowPlaying = QString();
    static bool listened = true;
    if(nowPlaying != track.title && !track.artist.isEmpty()) {
        nowPlaying = track.title;
        emit trackChanged(track.artist, track.song, track.totalSec);
        return;
    }
    if(track.file.startsWith(QLatin1String("http")))
        return;
    if(listened && ((track.currSec < track.totalSec/2 && track.totalSec < 8*60)
                    || (track.currSec < 4*60 && track.totalSec > 8*60))) {
        listened = false;
    }
    else if(!listened && (track.currSec > track.totalSec/2 ||
                          (track.currSec > 4*60 && track.totalSec > 8*60))) {
        listened = true;
        emit trackListened(track.artist,track.song,track.album,track.totalSec);
    }
}

QString PlayerInterface::cover() {
    if(track.file.startsWith(QLatin1String("http")))
        return QString();
    QString path = track.file;
    path.replace(QRegExp(QLatin1String("(.*)/(.*)")), QLatin1String("\\1"));
    QDir dir(path);
    dir.setNameFilters(QStringList{
                           QLatin1String("*.png"),
                           QLatin1String("*.jpg"),
                           QLatin1String("*.jpeg")});
    if(!dir.entryList().isEmpty())
        return path + QLatin1String("/") + dir.entryList().at(0);
    else
        return QLatin1String(":/images/nocover.png");
}

#ifdef BUILD_DBUS
QString PlayerInterface::artwork() {
    QString art = cover();
    return (art == QLatin1String(":/images/nocover.png")
            || art.isEmpty()) ? QString() : QLatin1String("file://") + art;
} // compliance method for MPRIS
#endif // BUILD_DBUS
