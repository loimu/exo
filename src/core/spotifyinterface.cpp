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

#include <QVector>
#include <QStringList>
#include <QProcess>
#include <QRegularExpression>
#include <QDBusReply>

#include "sysutils.h"
#include "spotifyinterface.h"

#define PLAYER_EXECUTABLE "spotify"

QT_BEGIN_NAMESPACE
Q_DBUS_EXPORT extern bool qt_dbus_metaobject_skip_annotations;
QT_END_NAMESPACE


SpotifyInterface::SpotifyInterface(QObject* parent) : PlayerInterface(parent),
    player(QStringLiteral(PLAYER_EXECUTABLE))
{
    if(SysUtils::findProcessId(player) < 0)  // check if player is running
        QProcess::startDetached(player, QStringList{});
    startTimer(1000);

    QT_PREPEND_NAMESPACE(qt_dbus_metaobject_skip_annotations) = true;
}

void SpotifyInterface::runServer() {
    if(SysUtils::findProcessId(player) < 0) {  // check if player is running
        QProcess p;
        p.start(player, QStringList{}, QIODevice::NotOpen);
        p.waitForFinished();
    }
}

PState SpotifyInterface::updateInfo() {
    QDBusInterface spotify{"org.mpris.MediaPlayer2.spotify", "/org/mpris/MediaPlayer2",
                           "org.mpris.MediaPlayer2.Player"};
    const QString status = spotify.property("PlaybackStatus").toString();
    if(status != QStringLiteral("Paused") && status != QStringLiteral("Playing")) {
        return PState::Offline;
    }

    const QVariantMap metadata = spotify.property("Metadata").toMap();

    track.artist = metadata["xesam:artist"].toString();
    track.title = metadata["xesam:title"].toString();
    track.album = metadata["xesam:album"].toString();
    track.file = QString();
    track.totalTime = QString();
    track.totalSec = metadata["mpris:length"].toULongLong() / 1000000;
    track.currSec = spotify.property("Position").toULongLong() / 1000000;
    track.caption = QString(QStringLiteral("%1 - %2")).arg(track.artist, track.title);

    return status.startsWith("Playing") ? PState::Play : PState::Pause;
}

const QString SpotifyInterface::id() const {
    return QStringLiteral("Spotify");
}

void SpotifyInterface::timerEvent(QTimerEvent* event) {
    Q_UNUSED(event);
    notify();
}
