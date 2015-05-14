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
#include <QDebug>
#include <QDBusMessage>
#include <QDBusConnection>

#include "../playerinterface.h"
#include "playerobject.h"

PlayerObject::PlayerObject(QObject *parent) : QDBusAbstractAdaptor(parent)
{
    player = PlayerInterface::instance();
    connect(player, SIGNAL(trackChanged(QString,QString,int)), SLOT(emitPropertiesChanged()));
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

QVariantMap PlayerObject::metadata() const {
    QVariantMap map;
    map["mpris:length"] = player->length() * 1000000;
    map["mpris:artUrl"] = "file://" + player->artwork();
    map["xesam:album"] = player->album();
    map["xesam:artist"] = QStringList() << player->artist();
    map["xesam:title"] = player->title();
    map["xesam:url"] = "file://" + player->url();
    return map;
}

QString PlayerObject::playbackStatus() const {
    if(player->state() == "PLAY")
        return "Playing";
    else if(player->state() == "PAUSE")
        return "Paused";
    return "Stopped";
}

qlonglong PlayerObject::position() const {
    return player->position() * 1000000;
}

void PlayerObject::emitPropertiesChanged() {
    QList<QByteArray> changedProps;
//    if(props["CanGoNext"] != canGoNext())
//        changedProps << "CanGoNext";
//    if(props["CanGoPrevious"] != canGoPrevious())
//        changedProps << "CanGoPrevious";
//    if(props["CanPause"] != canPause())
//        changedProps << "CanPause";
//    if(props["CanPlay"] != canPlay())
//        changedProps << "CanPlay";
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
    player->pause();
}

void PlayerObject::Previous() {
    player->prev();
}

void PlayerObject::Stop() {
    player->stop();
}

void PlayerObject::syncProperties() {
//    props["CanGoNext"] = canGoNext();
//    props["CanGoPrevious"] = canGoPrevious();
//    props["CanPause"] = canPause();
//    props["CanPlay"] = canPlay();
    props["PlaybackStatus"] = playbackStatus();
    props["Metadata"] = metadata();
}
