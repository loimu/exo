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

#include "config.h"

#include <QSettings>
#include <QNetworkProxyFactory>

#ifdef BUILD_LASTFM
#include "scrobblersettings.h"
#include "scrobbler.h"
#endif // BUILD_LASTFM

#include "trayicon.h"
#include "playerinterface.h"
#include "mocplayerinterface.h"
#include "exo.h"

Exo::Exo(int &argc, char **argv, bool useGui) : QApplication(argc, argv, useGui)
{
    QCoreApplication::setOrganizationName("exo");
    QCoreApplication::setApplicationName("eXo");
    QCoreApplication::setApplicationVersion("0.3");
    QApplication::setQuitOnLastWindowClosed(false);
    QNetworkProxyFactory::setUseSystemConfiguration(true);
    init(useGui);
}

Exo::~Exo() {
    if(settingsObject->value("player/quit").toBool())
       player->quit();
}

void Exo::init(bool useGui) {
    settingsObject = new QSettings(qApp->organizationName(),
                                   qApp->applicationName(), this);
    player = new MOCPlayerInterface(this);
#ifdef BUILD_LASTFM
    if(settingsObject->value("scrobbler/enabled").toBool())
        loadScrobbler();
#endif // BUILD_LASTFM
    if(useGui && QSystemTrayIcon::isSystemTrayAvailable()) {
        TrayIcon *trayIcon = new TrayIcon(this);
        trayIcon->hide();
#ifdef BUILD_LASTFM
        connect(trayIcon, SIGNAL(loadScrobbler()), SLOT(configureScrobbler()));
        connect(trayIcon, SIGNAL(unloadScrobbler()), SLOT(unloadScrobbler()));
#endif // BUILD_LASTFM
    }
}

Exo* Exo::app() {
    return (Exo*)qApp;
}

QSettings* Exo::settings() {
    return settingsObject;
}

#ifdef BUILD_LASTFM

void Exo::configureScrobbler() {
    if(!settingsObject->value("scrobbler/sessionkey").toBool()) {
        ScrobblerSettings *settingsDialog = new ScrobblerSettings(this);
        settingsDialog->show();
        connect(settingsDialog, SIGNAL(configured()), SLOT(enableScrobbler()));
    } else
        enableScrobbler();
}

void Exo::loadScrobbler() {
    if(scrobbler)
        return;
    scrobbler = new Scrobbler(this);
    connect(player, SIGNAL(trackChanged(QString, QString, int)),
            scrobbler, SLOT(init(QString, QString, int)));
    connect(player, SIGNAL(trackListened(QString, QString, QString, int)),
            scrobbler, SLOT(submit(QString, QString, QString, int)));
}

void Exo::unloadScrobbler() {
    settingsObject->setValue("scrobbler/enabled", false);
    if(scrobbler)
        scrobbler->deleteLater();
}

void Exo::enableScrobbler() {
    settingsObject->setValue("scrobbler/enabled", true);
    loadScrobbler();
}

#endif // BUILD_LASTFM
