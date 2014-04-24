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
#include "exo.h"

Exo::Exo(int &argc, char **argv, bool useGui, QString appName, QString orgName)
    : QApplication(argc, argv, useGui), m_pSettings(0) {
    m_pSettings = new QSettings(orgName, appName, this);
    m_pPlayer = new PlayerInterface(this);
    init(useGui);
}

Exo* Exo::app() {
    return (Exo*)qApp;
}

QSettings* Exo::settings() {
    return m_pSettings;
}

void Exo::init(bool useGui) {
    QSettings* settings = m_pSettings;
    if(!settings->value("scrobbler/disabled").toBool() &&
            settings->value("scrobbler/sessionkey").toBool())
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
    QSettings* settings = m_pSettings;
    if(!settings->value("scrobbler/sessionkey").toBool()) {
        ScrobblerSettings *settingsDialog = new ScrobblerSettings(this);
        settingsDialog->show();
        connect(settingsDialog, SIGNAL(configured()),
                this, SLOT(loadScrobbler()));
        connect(settingsDialog, SIGNAL(configured()),
                this, SLOT(enableScrobbler()));
    } else {
        enableScrobbler();
        loadScrobbler();
    }
}

void Exo::loadScrobbler() {
    if(!m_scrobbler) {
        m_scrobbler = new Scrobbler(this);
        connect(m_pPlayer, SIGNAL(trackChanged(QString, QString, int)),
                m_scrobbler, SLOT(init(QString, QString, int)));
        connect(m_pPlayer, SIGNAL(trackListened(QString, QString, QString, int)),
                m_scrobbler, SLOT(submit(QString, QString, QString, int)));
    }
}

void Exo::unloadScrobbler() {
    QSettings* settings = m_pSettings;
    settings->setValue("scrobbler/disabled", true);
    if(m_scrobbler) {
        m_scrobbler->deleteLater();
    }
}

void Exo::enableScrobbler() {
    QSettings* settings = m_pSettings;
    settings->setValue("scrobbler/disabled", false);
}
