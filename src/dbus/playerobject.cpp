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

#include <QStringList>
#include <QDBusMessage>
#include <QDBusConnection>

#include "playerobject.h"

PlayerObject::PlayerObject(QObject *parent) : QDBusAbstractAdaptor(parent),
    player(PlayerInterface::self()),
    track(PlayerInterface::self()->trackObject()),
    trackID("/org/exo/MediaPlayer2/Track/0")
{
    connect(player, SIGNAL(newStatus(State)),SLOT(emitPropsChanged(State)));
    connect(player, SIGNAL(newTrack()), SLOT(trackChanged()));
}

bool PlayerObject::canControl() const {
    return true;
}

bool PlayerObject::canGoNext() const {
    return true;
}

bool PlayerObject::canGoPrevious() const {
    return true;
}

bool PlayerObject::canPause() const {
    return true;
}

bool PlayerObject::canPlay() const {
    return true;
}

bool PlayerObject::canSeek() const {
    return !track->file.startsWith(QLatin1String("http"));
}

QVariantMap PlayerObject::metadata() const {
    QVariantMap map;
    map[QLatin1String("mpris:length")] = track->totalSec * 1000000;
    map[QLatin1String("mpris:artUrl")] = player->artwork();
    map[QLatin1String("mpris:trackid")] = QVariant::fromValue<QDBusObjectPath>(trackID);
    map[QLatin1String("xesam:album")] = track->album;
    map[QLatin1String("xesam:artist")] = QStringList() << track->artist;
    map[QLatin1String("xesam:title")] = track->song.isEmpty() ? track->title : track->song;
    QString uri = track->file;
    map[QLatin1String("xesam:url")] = uri.startsWith(
                QLatin1String("http")) ? uri : QLatin1String("file://") + uri;
    return map;
}

QString PlayerObject::playbackStatus() const {
    if(status == State::Play)
        return QLatin1String("Playing");
    else if(status == State::Pause)
        return QLatin1String("Paused");
    return QLatin1String("Stopped");
}

qlonglong PlayerObject::position() const {
    if(!track->file.startsWith(QLatin1String("http")))
        return track->currSec * 1000000;
}

double PlayerObject::volume() const {
    // dummy method - can't get the volume
    return 0.5;
}

void PlayerObject::setVolume(double value) {
    player->volume(value * 100);
}

void PlayerObject::trackChanged() {
    trackID = QDBusObjectPath(
                QString(QLatin1String("/org/exo/MediaPlayer2/Track/%1")).arg(qrand()));
    emitPropsChanged(State::Play);
}

void PlayerObject::emitPropsChanged(State st) {
    status = st;
    QList<QByteArray> changedProps;
    if(props[QLatin1String("CanSeek")] != canSeek())
        changedProps << QByteArray("CanSeek");
    if(props[QLatin1String("PlaybackStatus")] != playbackStatus())
        changedProps << QByteArray("PlaybackStatus");
    if(props[QLatin1String("Metadata")] != metadata())
        changedProps << QByteArray("Metadata");
    if(changedProps.isEmpty())
        return;
    syncProperties();
    QVariantMap map;
    for(QByteArray name : changedProps)
        map.insert(name, props.value(name));
    QDBusMessage msg = QDBusMessage::createSignal(
                QLatin1String("/org/mpris/MediaPlayer2"),
                QLatin1String("org.freedesktop.DBus.Properties"),
                QLatin1String("PropertiesChanged"));
    msg << "org.mpris.MediaPlayer2.Player";
    msg << map;
    msg << QStringList();
    QDBusConnection::sessionBus().send(msg);
}

void PlayerObject::Next() {
    player->next();
}

void PlayerObject::Pause() {
    player->pause();
}

void PlayerObject::Play() {
    player->play();
}

void PlayerObject::PlayPause() {
    player->playPause();
}

void PlayerObject::Previous() {
    player->prev();
}

void PlayerObject::Stop() {
    player->stop();
}

void PlayerObject::Seek(qlonglong Offset) {
    player->seek(Offset/1000000);
}

void PlayerObject::SetPosition(const QDBusObjectPath &TrackId, qlonglong Position) {
    if(trackID != TrackId)
        return;
    player->jump(Position/1000000);
}

void PlayerObject::OpenUri(const QString &Uri) {
    player->openUri(Uri);
}

void PlayerObject::syncProperties() {
    props[QLatin1String("CanGoNext")] = canGoNext();
    props[QLatin1String("CanGoPrevious")] = canGoPrevious();
    props[QLatin1String("CanPause")] = canPause();
    props[QLatin1String("CanPlay")] = canPlay();
    props[QLatin1String("CanSeek")] = canSeek();
    props[QLatin1String("PlaybackStatus")] = playbackStatus();
    props[QLatin1String("Metadata")] = metadata();
}
