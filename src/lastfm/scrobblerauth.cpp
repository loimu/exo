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

#include "scrobbler.h"
#include "scrobblerauth.h"

ScrobblerAuth::ScrobblerAuth(QObject* parent) : QObject(parent)
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
    params["method"] = QStringLiteral("auth.getMobileSession");
    params["username"] = username;
    params["authToken"] =
            lastfm::md5((username + lastfm::md5(password.toUtf8())).toUtf8());
    QNetworkReply* reply = lastfm::ws::post(params);
    connect(reply, &QNetworkReply::finished, this, [=] {
            if(!reply) {
                emit failed(tr("network error, try again"));
                return;
            }
            reply->deleteLater();
            lastfm::XmlQuery lfm = lastfm::XmlQuery();
            if(parseQuery(reply->readAll(), &lfm)) {
                /* save the session key and username */
                QSettings settings;
                settings.setValue(QStringLiteral("scrobbler/login"),
                                  lfm["session"]["name"].text());
                settings.setValue(QStringLiteral("scrobbler/sessionkey"),
                                  lfm["session"]["key"].text());
                settings.setValue(QStringLiteral("scrobbler/enabled"), true);
                emit configured();
            } else emit failed(tr("wrong data, try again"));
    }); // for more detailed error report handle error(NetworkError) signal
}

bool ScrobblerAuth::parseQuery(const QByteArray& data, lastfm::XmlQuery* query,
                               bool* connectionProblems) {
    const bool dataParsed = query->parse(data);
    if(connectionProblems)
        *connectionProblems = !dataParsed
            && query->parseError().enumValue() == lastfm::ws::MalformedResponse;
    return dataParsed;
}
