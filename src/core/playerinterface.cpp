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

#include "playerinterface.h"

#include <QDir>
#include <QDateTime>
#include <QSettings>
#include <QRandomGenerator>

#ifdef BUILD_LASTFM
#include "scrobbler.h"
#endif // BUILD_LASTFM


namespace PlayerInterfaceConstants
{
   // when the track is long enough we don't want to wait until half-time is passed
   constexpr int full = 6 * 60;  // full length of boundary condition time
   constexpr int half = full/2;  // half length ... (seconds)
}
using namespace PlayerInterfaceConstants;


PlayerInterface* PlayerInterface::object = nullptr;

PlayerInterface::PlayerInterface(QObject* parent) : QObject(parent)
{
    if(object)
        qFatal("PlayerInterface: only one instance is allowed");
    object = this;
}

void PlayerInterface::notify() {
    static PState state = PState::Offline;
    PState currentState = updateInfo();
    if(state != currentState) {
        emit newStatus(currentState);
        if(state == PState::Pause && currentState == PState::Play) {
            emit paused(getCover(), /* paused= */false);
        }
        else if(state == PState::Play && currentState == PState::Pause) {
            emit paused(getCover(), /* paused= */true);
        }
        state = currentState;
    }
    static QString nowPlaying;
#ifdef BUILD_LASTFM
    static QDateTime begin;
#endif // BUILD_LASTFM
    if(nowPlaying != track.caption) {
        nowPlaying = track.caption;
        if(track.caption.isEmpty()) {
            return;
        }
        track.trackId = QRandomGenerator::global()->generate();
        emit newTrack(getCover());
#ifdef BUILD_LASTFM
        begin = QDateTime::currentDateTimeUtc();
        if(auto scrobbler = Scrobbler::self()) {
            scrobbler->submit();
            if(track.isStream && !scrobbleStreams) {
                return;
            }
            if(currentState == PState::Play && !track.artist.isEmpty()) {
                scrobbler->init(track.artist, track.title,
                                track.album, track.totalSec);
            }
        }
#endif // BUILD_LASTFM
    }
#ifdef BUILD_LASTFM
    if(currentState != PState::Play || (track.isStream && !scrobbleStreams)) {
        return;
    }
    const qint64 delta = begin.secsTo(QDateTime::currentDateTimeUtc());
    qint64 currSec = -1;
    if(track.isStream) {
        track.totalSec = 2*60;  // workaround for short tracks during streams
        currSec = delta;
    } else {
        currSec = track.currSec;
    }
    if (currSec < 0) {
        return;
    }
    static bool listened = true;
    if(listened && ((currSec <= track.totalSec/2 && track.totalSec <= full)
                     || (currSec <= half && track.totalSec > full))) {
        listened = false;  // track started
    }
    else if(!listened && (currSec > track.totalSec/2
                           || (currSec > half && track.totalSec > full))) {
        listened = true;  // track listened
        const qint64 threshold =
            (track.totalSec > 2 * 60) ? 59 : track.totalSec/2 - 1;
        if(delta >= threshold) {
            if(auto scrobbler = Scrobbler::self()) {
                scrobbler->cache(track.artist, track.title,
                                 track.album, track.totalSec);
                scrobbler->submit();
            }
        }
    }
#endif // BUILD_LASTFM
}

QString PlayerInterface::getCover() {
    if(!track.isStream) {
        const QString path = track.file.section(QChar::fromLatin1('/'), 0, -2);
        QDir dir(path);
        dir.setNameFilters(QStringList{
                               QStringLiteral("*.png"),
                               QStringLiteral("*.jpg"),
                               QStringLiteral("*.jpeg")});
        if(!dir.entryList().isEmpty()) {
            return path + QChar::fromLatin1('/') + dir.entryList().at(0);
        }
    }
    return QString();
}
