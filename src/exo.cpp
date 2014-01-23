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

Exo::Exo(int &argc, char **argv) : QApplication(argc, argv) {
    m_player = new PlayerInterface(this);
    QSettings settings("exo", "eXo");
    settings.beginGroup(Scrobbler::settingsGroup);
    if(!settings.value("disabled").toBool() &&
            settings.value("sessionkey").toBool())
        loadScrobbler();
    if(QSystemTrayIcon::isSystemTrayAvailable()) {
        TrayIcon *trayicon = new TrayIcon(m_player);
        trayicon->hide();
        connect(trayicon, SIGNAL(loadScrobbler()),
                this, SLOT(configureScrobbler()));
        connect(trayicon, SIGNAL(unloadScrobbler()),
                this, SLOT(unloadScrobbler()));
    }
}

void Exo::configureScrobbler() {
    QSettings settings;
    settings.beginGroup(Scrobbler::settingsGroup);
    if(!settings.value("sessionkey").toBool()) {
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
        connect(m_player, SIGNAL(trackChanged(QString, QString, int)),
                m_scrobbler, SLOT(init(QString, QString, int)));
        connect(m_player, SIGNAL(trackListened(QString, QString, QString, int)),
                m_scrobbler, SLOT(submit(QString, QString, QString, int)));
    }
}

void Exo::unloadScrobbler() {
    QSettings settings;
    settings.beginGroup(Scrobbler::settingsGroup);
    settings.setValue("disabled", true);
    if(m_scrobbler) {
        m_scrobbler->deleteLater();
    }
}

void Exo::enableScrobbler() {
    QSettings settings;
    settings.beginGroup(Scrobbler::settingsGroup);
    settings.setValue("disabled", false);
}
