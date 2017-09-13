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
    track.caption.clear();
    State currentState = getInfo();
    static State state = Offline;
    if(state != currentState) {
        state = currentState;
#ifdef BUILD_DBUS
        emit newStatus(currentState);
#endif // BUILD_DBUS
        if(currentState == Offline)
            emit updateStatus(tr("Player isn't running."));
        else if(currentState == Stop) emit updateStatus(tr("Stopped"));
    }
    static QString nowPlaying = QString();
    if(nowPlaying != track.caption) {
        nowPlaying = track.caption;
        if(track.caption.isEmpty()) return;
        if(track.totalTime.isEmpty())
            emit updateStatus(track.caption, cover());
        else
            emit updateStatus(track.caption + QString(QLatin1String(" (%1)"))
                              .arg(track.totalTime), cover());
#ifdef BUILD_DBUS
        emit newTrack();
#endif // BUILD_DBUS
#ifdef BUILD_LASTFM
        if(currentState == Play && !track.artist.isEmpty())
            emit trackChanged(track.artist, track.title, track.totalSec);
#endif // BUILD_LASTFM
    }
#ifdef BUILD_LASTFM
    if(currentState != Play || track.isStream) return;
    static bool listened = true;
    if(listened && ((track.currSec < track.totalSec/2 && track.totalSec <= 8*60)
                || (track.currSec < 4*60 && track.totalSec > 8*60)))
        listened = false; // beginning
    else if(!listened && (track.currSec > track.totalSec/2
                || (track.currSec > 4*60 && track.totalSec > 8*60))) {
        listened = true; // ending
        emit trackListened(track.artist, track.title,
                           track.album, track.totalSec);
    }
#endif // BUILD_LASTFM
}

QString PlayerInterface::cover() {
    if(track.isStream)
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
    return art.endsWith(QLatin1String("/nocover.png")) ?
                QString() : QLatin1String("file://") + art;
} // compliance method for MPRIS
#endif // BUILD_DBUS
