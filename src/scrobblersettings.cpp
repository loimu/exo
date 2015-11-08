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

#include <QSettings>

#include <lastfm/ws.h>
#include <lastfm/misc.h>
#include <lastfm/XmlQuery.h>

#include "exo.h"
#include "scrobbler.h"
#include "scrobblersettings.h"
#include "ui_scrobblersettings.h"

ScrobblerSettings::ScrobblerSettings(QObject *parent) : ui(new Ui::ScrobblerSettings) {
    ui->setupUi(this);
}

ScrobblerSettings::~ScrobblerSettings() {
    delete ui;
}

void ScrobblerSettings::on_buttonBox_accepted() {
    if(ui->usernameLineEdit->text().size() > 1 &&
            ui->passwordLineEdit->text().size() > 1) {
        QString username = ui->usernameLineEdit->text();
        QString password = ui->passwordLineEdit->text();
        lastfm::ws::ApiKey = Scrobbler::apiKey;
        lastfm::ws::SharedSecret = Scrobbler::secret;
        auth(username, password);
    } else
        ui->label->setText(tr("enter username and password"));
}

void ScrobblerSettings::on_usernameLineEdit_returnPressed() {
    on_buttonBox_accepted();
}

void ScrobblerSettings::on_passwordLineEdit_returnPressed() {
    on_buttonBox_accepted();
}

void ScrobblerSettings::on_usernameLineEdit_textChanged() {
    ui->label->setText("");
}

void ScrobblerSettings::on_passwordLineEdit_textChanged() {
    ui->label->setText("");
}

void ScrobblerSettings::on_buttonBox_rejected() {
    this->close();
}

void ScrobblerSettings::auth(const QString& username, const QString& password) {
    QMap<QString, QString> params;
    params["method"] = "auth.getMobileSession";
    params["username"] = username;
    params["authToken"] =
            lastfm::md5((username +lastfm::md5(password.toUtf8())).toUtf8());
    QNetworkReply* reply = lastfm::ws::post(params);
    connect(reply, SIGNAL(finished()), SLOT(authReplyFinished()));
    // for more detailed error report handle error(NetworkError) signal
}

void ScrobblerSettings::authReplyFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) {
        ui->label->setText(tr("network error, try again"));
        return;
    }
    reply->deleteLater();
    // Parse the reply
    lastfm::XmlQuery lfm = lastfm::XmlQuery();
    if (parseQuery(reply->readAll(), &lfm)) {
        lastfm::ws::Username = lfm["session"]["name"].text();
        lastfm::ws::SessionKey = lfm["session"]["key"].text();
        // Save the session key
        Exo::settings->setValue("scrobbler/login", lastfm::ws::Username);
        Exo::settings->setValue("scrobbler/sessionkey", lastfm::ws::SessionKey);
        emit configured(true);
        this->close();
    } else
        ui->label->setText(tr("wrong data, try again"));
}

bool ScrobblerSettings::parseQuery(const QByteArray& data, lastfm::XmlQuery* query,
                                   bool* connectionProblems) {
    const bool dataParsed = query->parse(data);
    if(connectionProblems)
        *connectionProblems = !dataParsed && query->parseError().enumValue() ==
                lastfm::ws::MalformedResponse;
    return dataParsed;
}
