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
#include <lastfm/misc.h>
#include <lastfm/XmlQuery.h>
#include <lastfm/Artist.h>
#include <lastfm/Audioscrobbler.h>
#include <lastfm/Track.h>

#include <QDebug>
#include <QSettings>

#include "playerinterface.h"
#include "scrobbler.h"

Scrobbler::Scrobbler(QObject *parent, QSettings *settings,
                     PlayerInterface* player) : QObject(parent) {
    m_player = player;
    m_settings = settings;

    lastfm::ws::ApiKey = "75ca28a33e04af35b315c086736a6e7c";
    lastfm::ws::SharedSecret = "a341d91dcf4b4ed725b72f27f1e4f2ef";
    QString username = m_settings->value("scrobbler/login").toString();
    QString password = m_settings->value("scrobbler/password").toString();
    lastfm::ws::Username = username;

    QString key = m_settings->value("scrobbler/sessionkey").toString();
    lastfm::ws::SessionKey = key;
    if(!key.length() > 0)
        auth(username, password);

    connect(m_player, SIGNAL(trackChanged()), this, SLOT(init()));
    connect(m_player, SIGNAL(trackListened()), this, SLOT(submit()));

    as = new lastfm::Audioscrobbler("eXo");
}

void Scrobbler::auth(const QString& username, const QString& password) {
    QMap<QString, QString> params;
    params["method"] = "auth.getMobileSession";
    params["username"] = username;
    params["authToken"] =
            lastfm::md5((username +lastfm::md5(password.toUtf8())).toUtf8());

    QNetworkReply* reply = lastfm::ws::post(params);
    connect(reply, SIGNAL(finished()), SLOT(authReplyFinished()));
    // If we need more detailed error report, handle error(NetworkError) signal
}

void Scrobbler::authReplyFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply)
        return;
    reply->deleteLater();

    // Parse the reply
    lastfm::XmlQuery lfm(EmptyXmlQuery());
    if (ParseQuery(reply->readAll(), &lfm)) {
        lastfm::ws::Username = lfm["session"]["name"].text();
        lastfm::ws::SessionKey = lfm["session"]["key"].text();

        // Save the session key
        m_settings->setValue("scrobbler/login", lastfm::ws::Username);
        m_settings->setValue("scrobbler/sessionkey", lastfm::ws::SessionKey);
    } else
        return;
}

void Scrobbler::init() {
    lastfm::MutableTrack t;
    if(m_player->m_list.at(3).isEmpty()) {
        QString artist, title;
        artist = title = m_player->m_list.at(4);
        artist.replace(QRegExp("^([\\s\\w\\(\\)]+)\\s-\\s.*"), "\\1");
        title.replace(QRegExp("^[\\s\\w\\(\\)]+\\s-\\s(.*)"), "\\1");
        t.setArtist(artist);
        t.setTitle(title);
        t.setDuration(8*60);
    }
    else if(m_player->m_list.at(4).isEmpty()) {
        qDebug() << "empty track";
    }
    else {
        t.setArtist(m_player->m_list.at(3));
        t.setTitle(m_player->m_list.at(4));
        t.setDuration(m_player->m_list.at(8).toInt());
    }

    as->nowPlaying(t);
}

void Scrobbler::submit() {
    lastfm::MutableTrack t;
    t.setArtist(m_player->m_list.at(3));
    t.setTitle(m_player->m_list.at(4));
    t.setAlbum(m_player->m_list.at(5));
    t.setDuration(m_player->m_list.at(8).toInt());
    t.stamp(); //sets track start time

    as->cache(t);
    as->submit();
}

lastfm::XmlQuery Scrobbler::EmptyXmlQuery() {
  return lastfm::XmlQuery();
}

bool Scrobbler::ParseQuery(const QByteArray& data, lastfm::XmlQuery* query,
                           bool* connection_problems) {
    const bool ret = query->parse(data);

    if(connection_problems) {
        *connection_problems = !ret && query->parseError().enumValue() ==
                lastfm::ws::MalformedResponse;
    }

    return ret;
}
