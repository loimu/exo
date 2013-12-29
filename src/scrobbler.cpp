/* ========================================================================
*    Copyright (C) 2013 Blaze <blaze@jabster.pl>
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

#include <lastfm/ws.h>
#include <lastfm/Audioscrobbler.h>
#include <lastfm/Track.h>

#include <QSettings>

#include "playerinterface.h"
#include "scrobbler.h"

Scrobbler::Scrobbler(QObject *parent, PlayerInterface* player,
                     QSettings *settings) : QObject(parent) {
    m_settings = settings;
    lastfm::ws::ApiKey = "75ca28a33e04af35b315c086736a6e7c";
    lastfm::ws::SharedSecret = "a341d91dcf4b4ed725b72f27f1e4f2ef";
    QString username = m_settings->value("scrobbler/login").toString();
    lastfm::ws::Username = username;
    QString key = m_settings->value("scrobbler/sessionkey").toString();
    lastfm::ws::SessionKey = key;
    connect(player, SIGNAL(trackChanged(QString, QString, int)),
            this, SLOT(init(QString, QString, int)));
    connect(player, SIGNAL(trackListened(QString, QString, QString, int)),
            this, SLOT(submit(QString, QString, QString, int)));
    as = new lastfm::Audioscrobbler("eXo");
}

Scrobbler::~Scrobbler() {
    delete as;
}

void Scrobbler::init(QString artist, QString title, int totalSec) {
    lastfm::MutableTrack t;
    t.setArtist(artist);
    t.setTitle(title);
    t.setDuration(totalSec);
    if(m_settings->value("scrobbler/enabled").toBool())
        as->nowPlaying(t);
}

void Scrobbler::submit(QString artist, QString title,
                       QString album,int totalSec) {
    lastfm::MutableTrack t;
    t.setArtist(artist);
    t.setTitle(title);
    t.setAlbum(album);
    t.setDuration(totalSec);
    t.stamp(); //sets track start time
    if(m_settings->value("scrobbler/enabled").toBool()) {
        as->cache(t);
        as->submit();
    }
}
