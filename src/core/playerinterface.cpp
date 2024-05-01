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

#include <QDir>
#include <QDateTime>

#ifdef BUILD_LASTFM
  #include "scrobbler.h"
#endif // BUILD_LASTFM
#include "playerinterface.h"


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
        if(auto scrobbler = Scrobbler::self()) {
            if(currentState == PState::Play && !track.artist.isEmpty()) {
                scrobbler->init(track.artist, track.title,
                                track.album, track.totalSec);
                scrobbler->submit();
            }
        }
#endif // BUILD_LASTFM
    }
#ifdef BUILD_LASTFM
    if(currentState != PState::Play || track.isStream) return;
    static bool listened = true;
    static QDateTime threshold;
    if(listened && ((track.currSec <= track.totalSec/2 && track.totalSec <= full)
                    || (track.currSec <= half && track.totalSec > full))) {
        listened = false; // beginning
        threshold = QDateTime::currentDateTime()
                .addSecs((track.totalSec > 2 * 60) ? 59 : track.totalSec/2 - 1);
    }
    else if(!listened && (track.currSec > track.totalSec/2
                          || (track.currSec > half && track.totalSec > full))) {
        listened = true; // ending
        if(QDateTime::currentDateTime() >= threshold)
            if(auto scrobbler = Scrobbler::self()) {
                scrobbler->cache(track.artist, track.title,
                                 track.album, track.totalSec);
                scrobbler->submit();
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
        if(!dir.entryList().isEmpty())
            return path + QChar::fromLatin1('/') + dir.entryList().at(0);
    }
    return QString();
}
