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

#include "config.h"

#include <QSettings>
#include <lastfm5/ws.h>
#include <lastfm5/misc.h>
#include <lastfm5/XmlQuery.h>

#include "lastfm/scrobbler.h"
#include "scrobblerauth.h"

ScrobblerAuth::ScrobblerAuth(QObject *parent) : QObject(parent)
{
    lastfm::ws::ApiKey = Scrobbler::apiKey;
    lastfm::ws::SharedSecret = Scrobbler::secret;
}

void ScrobblerAuth::auth(const QString& username, const QString& password) {
    if(username.isEmpty() || password.isEmpty()) {
        emit failed(tr("enter username and password"));
        return;
    }
    QMap<QString, QString> params;
    params["method"] = "auth.getMobileSession";
    params["username"] = username;
    params["authToken"] =
            lastfm::md5((username +lastfm::md5(password.toUtf8())).toUtf8());
    QNetworkReply* reply = lastfm::ws::post(params);
    connect(reply, &QNetworkReply::finished,
            this, &ScrobblerAuth::authReplyFinished);
    // for more detailed error report handle error(NetworkError) signal
}

void ScrobblerAuth::authReplyFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) {
        emit failed(tr("network error, try again"));
        return;
    }
    reply->deleteLater();
    // Parse the reply
    lastfm::XmlQuery lfm = lastfm::XmlQuery();
    if (parseQuery(reply->readAll(), &lfm)) {
        lastfm::ws::Username = lfm["session"]["name"].text();
        lastfm::ws::SessionKey = lfm["session"]["key"].text();
        // Save the session key
        QSettings settings;
        settings.setValue(QStringLiteral("scrobbler/login"),
                          lastfm::ws::Username);
        settings.setValue(QStringLiteral("scrobbler/sessionkey"),
                          lastfm::ws::SessionKey);
        settings.setValue(QStringLiteral("scrobbler/enabled"), true);
        emit configured();
    } else
        emit failed(tr("wrong data, try again"));
}

bool ScrobblerAuth::parseQuery(const QByteArray& data, lastfm::XmlQuery* query,
                                   bool* connectionProblems) {
    const bool dataParsed = query->parse(data);
    if(connectionProblems)
        *connectionProblems = !dataParsed && query->parseError().enumValue() ==
                lastfm::ws::MalformedResponse;
    return dataParsed;
}
