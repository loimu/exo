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

#include <QTimerEvent>
#include <QDir>
#include <QProcess>

#include "playerinterface.h"


QString Process::getOutput(const QString& program, const QStringList& options) {
    QProcess proc;
    proc.start(program, options);
    proc.waitForFinished(-1);
    return QString::fromUtf8(proc.readAllStandardOutput());
}

bool Process::execute(const QString& program, const QStringList& options) {
    QProcess proc;
    return proc.startDetached(program, options);
}

QStringList Process::detect(const QStringList& apps) {
    QProcess proc;
    proc.start(QStringLiteral("which"), apps);
    proc.waitForFinished(-1);
    return QString::fromUtf8(proc.readAllStandardOutput())
            .split(QStringLiteral("\n"), QString::SkipEmptyParts);
}


PlayerInterface* PlayerInterface::object = nullptr;
PITrack* PlayerInterface::ptrack = nullptr;

PlayerInterface::PlayerInterface(QObject* parent) : QObject(parent)
{
    if(object)
        qFatal("PlayerInterface: only one instance is allowed");
    object = this;
    ptrack = &track;
}

void PlayerInterface::timerEvent(QTimerEvent* event) {
    track.caption.clear();
    State currentState = getInfo();
    static State state = Offline;
    if(state != currentState) {
        state = currentState;
        emit newStatus(currentState);
    }
    static QString nowPlaying = QString();
    if(nowPlaying != track.caption) {
        nowPlaying = track.caption;
        if(track.caption.isEmpty()) return;
        getCover();
        emit newTrack();
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

void PlayerInterface::getCover() {
    track.cover = QString();
    if(!track.isStream) {
        QString path = track.file;
        path.replace(QRegExp(
                         QStringLiteral("(.*)/(.*)")), QStringLiteral("\\1"));
        QDir dir(path);
        dir.setNameFilters(QStringList{
                               QStringLiteral("*.png"),
                               QStringLiteral("*.jpg"),
                               QStringLiteral("*.jpeg")});
        if(!dir.entryList().isEmpty())
            track.cover = path + QChar::fromLatin1('/') + dir.entryList().at(0);
    }
}
