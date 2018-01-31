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

#include <QStringList>
#include <QDBusMessage>
#include <QDBusConnection>

#include "playerobject.h"

PlayerObject::PlayerObject(QObject* parent) : QDBusAbstractAdaptor(parent),
    player(PlayerInterface::self()),
    track(PlayerInterface::getTrack()),
    trackID("/org/exo/MediaPlayer2/Track/0")
{
    connect(player, &PlayerInterface::newStatus,
            this, &PlayerObject::emitPropsChanged);
    connect(player, &PlayerInterface::newTrack,
            this, &PlayerObject::trackChanged);
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
    return !track->isStream;
}

QVariantMap PlayerObject::metadata() const {
    QVariantMap map;
    map.insert(QStringLiteral("mpris:length"), track->totalSec * 1000000);
    map.insert(QStringLiteral("mpris:artUrl"), cover);
    map.insert(QStringLiteral("mpris:trackid"),
               QVariant::fromValue<QDBusObjectPath>(trackID));
    map.insert(QStringLiteral("xesam:album"), track->album);
    map.insert(QStringLiteral("xesam:artist"), QStringList() << track->artist);
    map.insert(QStringLiteral("xesam:title"),
               track->title.isEmpty() ? track->caption : track->title);
    map.insert(QStringLiteral("xesam:url"),
               track->isStream ? track->file : QLatin1String("file://")
                                 + track->file);
    return map;
}

QString PlayerObject::playbackStatus() const {
    if(status == PIState::Play)
        return QStringLiteral("Playing");
    else if(status == PIState::Pause)
        return QStringLiteral("Paused");
    return QStringLiteral("Stopped");
}

qlonglong PlayerObject::position() const {
        return track->isStream ? 0 : track->currSec * 1000000;
}

double PlayerObject::volume() const {
    // dummy method - can't get the volume
    return 0.5;
}

void PlayerObject::setVolume(double value) {
    player->volume(value * 100);
}

void PlayerObject::trackChanged(const QString& coverString) {
    cover = coverString.isEmpty()
            ? coverString : QLatin1String("file://") + coverString;
    trackID = QDBusObjectPath(
                QString(QLatin1String("/org/exo/MediaPlayer2/Track/%1"))
                .arg(qrand()));
    emitPropsChanged(PIState::Play);
}

void PlayerObject::emitPropsChanged(PIState st) {
    status = st;
    QList<QByteArray> changedProps;
    if(props.value(QStringLiteral("CanSeek")) != canSeek())
        changedProps << QByteArray("CanSeek");
    if(props.value(QStringLiteral("PlaybackStatus")) != playbackStatus())
        changedProps << QByteArray("PlaybackStatus");
    if(props.value(QStringLiteral("Metadata")) != metadata())
        changedProps << QByteArray("Metadata");
    if(changedProps.isEmpty())
        return;
    syncProperties();
    QVariantMap map;
    for(QByteArray& name : changedProps)
        map.insert(name, props.value(name));
    QDBusMessage msg = QDBusMessage::createSignal(
                QStringLiteral("/org/mpris/MediaPlayer2"),
                QStringLiteral("org.freedesktop.DBus.Properties"),
                QStringLiteral("PropertiesChanged"));
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
    player->seek(Offset / 1000000);
}

void PlayerObject::SetPosition(
        const QDBusObjectPath &TrackId, qlonglong Position) {
    if(trackID != TrackId)
        return;
    player->jump(Position / 1000000);
}

void PlayerObject::OpenUri(const QString &Uri) {
    player->openUri(Uri);
}

void PlayerObject::syncProperties() {
    props.insert(QStringLiteral("CanGoNext"), canGoNext());
    props.insert(QStringLiteral("CanGoPrevious"), canGoPrevious());
    props.insert(QStringLiteral("CanPause"), canPause());
    props.insert(QStringLiteral("CanPlay"), canPlay());
    props.insert(QStringLiteral("CanSeek"), canSeek());
    props.insert(QStringLiteral("PlaybackStatus"), playbackStatus());
    props.insert(QStringLiteral("Metadata"), metadata());
}
