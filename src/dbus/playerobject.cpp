/* ========================================================================
*    Copyright (C) 2013-2015 Blaze <blaze@open.by>
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
    player(PlayerInterface::instance()),
    track(PlayerInterface::instance()->trackObject()),
    trackID("/org/exo/MediaPlayer2/Track/0")
{
    connect(player, SIGNAL(newStatus(QString)),SLOT(emitPropsChanged(QString)));
    connect(player, SIGNAL(newTrack()), SLOT(trackChanged()));
}

PlayerObject::~PlayerObject()
{
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
    return !track->file.startsWith("http");
}

QVariantMap PlayerObject::metadata() const {
    QVariantMap map;
    map["mpris:length"] = track->totalSec * 1000000;
    map["mpris:artUrl"] = player->artwork();
    map["mpris:trackid"] = QVariant::fromValue<QDBusObjectPath>(trackID);
    map["xesam:album"] = track->album;
    map["xesam:artist"] = QStringList() << track->artist;
    map["xesam:title"] = track->song.isEmpty() ? track->title : track->song;
    QString uri = track->file;
    map["xesam:url"] = uri.startsWith("http") ? uri : "file://" + uri;
    return map;
}

QString PlayerObject::playbackStatus() const {
    if(status.startsWith("play", Qt::CaseInsensitive))
        return "Playing";
    else if(status.startsWith("pause", Qt::CaseInsensitive))
        return "Paused";
    return "Stopped";
}

qlonglong PlayerObject::position() const {
    if(!track->file.startsWith("http"))
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
    trackID = QDBusObjectPath(QString("/org/exo/MediaPlayer2/Track/%1").arg(qrand()));
    emitPropsChanged("PLAY");
}

void PlayerObject::emitPropsChanged(QString st) {
    status = st;
    QList<QByteArray> changedProps;
    if(props["CanSeek"] != canSeek())
        changedProps << "CanSeek";
    if(props["PlaybackStatus"] != playbackStatus())
        changedProps << "PlaybackStatus";
    if(props["Metadata"] != metadata())
        changedProps << "Metadata";
    if(changedProps.isEmpty())
        return;
    syncProperties();
    QVariantMap map;
    foreach(QByteArray name, changedProps)
        map.insert(name, props.value(name));
    QDBusMessage msg = QDBusMessage::createSignal("/org/mpris/MediaPlayer2",
                                                  "org.freedesktop.DBus.Properties",
                                                  "PropertiesChanged");
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
    props["CanGoNext"] = canGoNext();
    props["CanGoPrevious"] = canGoPrevious();
    props["CanPause"] = canPause();
    props["CanPlay"] = canPlay();
    props["CanSeek"] = canSeek();
    props["PlaybackStatus"] = playbackStatus();
    props["Metadata"] = metadata();
}
