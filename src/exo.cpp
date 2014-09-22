/* ========================================================================
*    Copyright (C) 2013-2014 Blaze <blaze@jabster.pl>
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

#include "scrobblersettings.h"
#include "scrobbler.h"
#include "trayicon.h"
#include "playerinterface.h"
#include "mocplayerinterface.h"
#include "exo.h"

Exo::Exo(int &argc, char **argv, bool useGui, QString appName, QString orgName)
    : QApplication(argc, argv, useGui), settingsObject(0) {
    settingsObject = new QSettings(orgName, appName, this);
    player = new MOCPlayerInterface(this);
    init(useGui);
}

Exo::~Exo() {
    if(settingsObject->value("player/quit").toBool())
       player->quit();
}

Exo* Exo::app() {
    return (Exo*)qApp;
}

QSettings* Exo::settings() {
    return settingsObject;
}

void Exo::init(bool useGui) {
    if(settingsObject->value("scrobbler/enabled").toBool())
        loadScrobbler();
    if(useGui && QSystemTrayIcon::isSystemTrayAvailable()) {
        TrayIcon *trayIcon = new TrayIcon();
        trayIcon->hide();
        connect(trayIcon, SIGNAL(loadScrobbler()),
                this, SLOT(configureScrobbler()));
        connect(trayIcon, SIGNAL(unloadScrobbler()),
                this, SLOT(unloadScrobbler()));
    }
}

void Exo::configureScrobbler() {
    if(!settingsObject->value("scrobbler/sessionkey").toBool()) {
        ScrobblerSettings *settingsDialog = new ScrobblerSettings(this);
        settingsDialog->show();
        connect(settingsDialog, SIGNAL(configured()),
                this, SLOT(enableScrobbler()));
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
