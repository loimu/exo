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

Scrobbler::Scrobbler(QObject *parent, QSettings *settings, PlayerInterface* player) : QObject(parent) {
    m_player = player;
    m_settings = settings;

    lastfm::ws::ApiKey = "75ca28a33e04af35b315c086736a6e7c";
    lastfm::ws::SharedSecret = "a341d91dcf4b4ed725b72f27f1e4f2ef";
    QString username = m_settings->value("scrobbler/login").toString();
    QString password = m_settings->value("scrobbler/password").toString();
    lastfm::ws::Username = username;

    auth(username, password);

    connect(m_player, SIGNAL(trackChanged()), this, SLOT(init()));
    connect(m_player, SIGNAL(trackListened()), this, SLOT(submit()));

    as = new lastfm::Audioscrobbler("eXo");
}

void Scrobbler::auth(const QString& username, const QString& password) {
    QMap<QString, QString> params;
    params["method"] = "auth.getMobileSession";
    params["username"] = username;
    params["authToken"] = lastfm::md5((username + lastfm::md5(password.toUtf8())).toUtf8());

    QNetworkReply* reply = lastfm::ws::post(params);
    connect(reply, SIGNAL(finished()), SLOT(authReplyFinished()));
    // If we need more detailed error reporting, handle error(NetworkError) signal
}

void Scrobbler::authReplyFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        emit AuthenticationComplete(false);
        return;
    }
    reply->deleteLater();

    // Parse the reply
    lastfm::XmlQuery lfm(EmptyXmlQuery());
    if (ParseQuery(reply->readAll(), &lfm)) {
        lastfm::ws::Username = lfm["session"]["name"].text();
        lastfm::ws::SessionKey = lfm["session"]["key"].text();

        // Save the session key
        m_settings->setValue("scrobbler/login", lastfm::ws::Username);
        m_settings->setValue("scrobbler/sessionkey", lastfm::ws::SessionKey);
    } else {
        emit AuthenticationComplete(false);
        return;
    }
    emit AuthenticationComplete(true);
}

void Scrobbler::init() {
    lastfm::MutableTrack t;
    t.setArtist(m_player->m_list.at(3));
    t.setTitle(m_player->m_list.at(4));
    t.setDuration(m_player->m_list.at(8).toInt()/2);

    as->nowPlaying(t);
}

void Scrobbler::submit() {
    lastfm::MutableTrack t;
    t.setArtist(m_player->m_list.at(3));
    t.setTitle(m_player->m_list.at(4));
    t.setAlbum(m_player->m_list.at(5));
    t.setDuration(m_player->m_list.at(8).toInt()/2);
    t.stamp(); //sets track start time

    as->cache(t);
    as->submit();
}

lastfm::XmlQuery Scrobbler::EmptyXmlQuery() {
  return lastfm::XmlQuery();
}

bool Scrobbler::ParseQuery(const QByteArray& data, lastfm::XmlQuery* query, bool* connection_problems) {
  const bool ret = query->parse(data);

  if (connection_problems) {
    *connection_problems =
        !ret && query->parseError().enumValue() == lastfm::ws::MalformedResponse;
  }

  return ret;
}
