/* ========================================================================
*    Copyright (C) 2013-2019 Blaze <blaze@vivaldi.net>
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

#include <QDir>
#include <QProcess>
#include <QDateTime>

#include "playerinterface.h"


QStringList Process::detect(const QStringList& apps) {
    QProcess proc;
    proc.start(QStringLiteral("which"), apps);
    proc.waitForFinished();
    return QString::fromUtf8(proc.readAllStandardOutput())
            .split(QStringLiteral("\n"), QString::SkipEmptyParts);
}


PlayerInterface* PlayerInterface::object = nullptr;

PlayerInterface::PlayerInterface(QObject* parent) : QObject(parent)
{
    if(object)
        qFatal("PlayerInterface: only one instance is allowed");
    object = this;
}

bool PlayerInterface::isPlayerRunning(const QString &player) {
    QProcess proc;
    proc.start(QStringLiteral("pidof"), QStringList{player});
    proc.waitForFinished();
    return !QString::fromUtf8(proc.readAllStandardOutput()).isEmpty();
}

void PlayerInterface::notify() {
    track.caption.clear();
    static PState state = PState::Offline;
    PState currentState = updateInfo();
    if(state != currentState) {
        state = currentState;
        emit newStatus(currentState);
    }
    static QString nowPlaying;
    if(nowPlaying != track.caption) {
        nowPlaying = track.caption;
        if(track.caption.isEmpty()) return;
        emit newTrack(getCover());
#ifdef BUILD_LASTFM
        if(currentState == PState::Play && !track.artist.isEmpty())
            emit trackChanged(track.artist, track.title, track.totalSec);
#endif // BUILD_LASTFM
    }
#ifdef BUILD_LASTFM
    if(currentState != PState::Play || track.isStream) return;
    static bool listened = true;
    static QDateTime threshold;
    if(listened && ((track.currSec < track.totalSec/2 && track.totalSec <= 8*60)
                    || (track.currSec < 4*60 && track.totalSec > 8*60))) {
        listened = false; // beginning
        threshold = QDateTime::currentDateTime()
                .addSecs(track.totalSec > 2*60 ? 60 : track.totalSec/2);
    }
    else if(!listened && (track.currSec > track.totalSec/2
                          || (track.currSec > 4*60 && track.totalSec > 8*60))) {
        listened = true; // ending
        if(QDateTime::currentDateTime() > threshold)
            emit trackListened(track.artist, track.title,
                               track.album, track.totalSec);
    }
#endif // BUILD_LASTFM
}

QString PlayerInterface::getCover() {
    if(!track.isStream) {
        QString path = track.file.section(QChar::fromLatin1('/'), 0, -2);
        QDir dir(path);
        dir.setNameFilters(QStringList{
                               QStringLiteral("*.png"),
                               QStringLiteral("*.jpg"),
                               QStringLiteral("*.jpeg")});
        if(!dir.entryList().isEmpty())
            return path + QChar::fromLatin1('/') + dir.entryList().at(0);
    }
    return QString();
}
