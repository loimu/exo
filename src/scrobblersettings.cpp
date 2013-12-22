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

#include <QSettings>
#include <QMessageBox>

#include <lastfm/ws.h>
#include <lastfm/misc.h>
#include <lastfm/XmlQuery.h>

#include "scrobblersettings.h"
#include "ui_scrobblersettings.h"

ScrobblerSettings::ScrobblerSettings(QSettings *settings)
    : ui(new Ui::ScrobblerSettings) {

    ui->setupUi(this);
    m_settings = settings;
}

ScrobblerSettings::~ScrobblerSettings() {
    delete ui;
}

void ScrobblerSettings::on_buttonBox_accepted() {
    if(ui->usernameLineEdit && ui->passwordLineEdit) {
        m_settings->setValue("scrobbler/enabled", true);
        m_settings->setValue("scrobbler/configured", true);
        QString username = ui->usernameLineEdit->text();
        QString password = ui->passwordLineEdit->text();
        lastfm::ws::ApiKey = "75ca28a33e04af35b315c086736a6e7c";
        lastfm::ws::SharedSecret = "a341d91dcf4b4ed725b72f27f1e4f2ef";
        auth(username, password);

        QMessageBox msgBox;
        msgBox.setText(tr("Please restart application for the changes to take"
                          " effect."));
        msgBox.exec();
    }
    this->close();
}

void ScrobblerSettings::on_usernameLineEdit_returnPressed() {
    on_buttonBox_accepted();
}

void ScrobblerSettings::on_passwordLineEdit_returnPressed() {
    on_buttonBox_accepted();
}

void ScrobblerSettings::on_buttonBox_rejected() {
    m_settings->setValue("scrobbler/configured", true);
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
    // If we need more detailed error report, handle error(NetworkError) signal
}

void ScrobblerSettings::authReplyFinished() {
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

lastfm::XmlQuery ScrobblerSettings::EmptyXmlQuery() {
    return lastfm::XmlQuery();
}

bool ScrobblerSettings::ParseQuery(const QByteArray& data, lastfm::XmlQuery* query,
                                   bool* connectionProblems) {
    const bool dataParsed = query->parse(data);

    if(connectionProblems)
        *connectionProblems = !dataParsed && query->parseError().enumValue() ==
                lastfm::ws::MalformedResponse;

    return dataParsed;
}
