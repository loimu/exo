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

#ifdef BUILD_DBUS
#include "dbus/dbus.h"
#endif // BUILD_DBUS

#ifdef BUILD_LASTFM
#include "scrobblersettings.h"
#include "scrobbler.h"
#endif // BUILD_LASTFM

#include "trayicon.h"
#include "playerinterface.h"

#ifdef USE_CMUS
#include "cmusinterface.h"
#else // USE_CMUS
#include "mocplayerinterface.h"
#endif // USE_CMUS

#include "exo.h"

Exo::Exo(int &argc, char **argv, bool useGui) : QApplication(argc, argv, useGui)
{
    QCoreApplication::setOrganizationName("exo");
    QCoreApplication::setApplicationName("eXo");
    QCoreApplication::setApplicationVersion("0.4");
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
#ifdef USE_CMUS
    player = new CmusInterface(this);
#else // USE_CMUS
    player = new MOCPlayerInterface(this);
#endif // USE_CMUS

#ifdef BUILD_DBUS
    new DBus(this);
#endif // BUILD_DBUS

#ifdef BUILD_LASTFM
    if(settingsObject->value("scrobbler/enabled").toBool() &&
            settingsObject->value("scrobbler/sessionkey").toBool())
        loadScrobbler();
#endif // BUILD_LASTFM

    if(useGui && QSystemTrayIcon::isSystemTrayAvailable()) {
        TrayIcon *trayIcon = new TrayIcon(this);
        trayIcon->hide();
    }
}

Exo* Exo::app() {
    return static_cast<Exo*>qApp;
}

QSettings* Exo::settings() {
    return settingsObject;
}

void Exo::showLyricsWindow() {
    emit lyricsWindow();
}

#ifdef BUILD_LASTFM
void Exo::configureScrobbler() {
    if(!settingsObject->value("scrobbler/sessionkey").toBool()) {
        ScrobblerSettings *settingsDialog = new ScrobblerSettings(this);
        settingsDialog->show();
        connect(settingsDialog, SIGNAL(configured()), SLOT(loadScrobbler()));
    } else
        loadScrobbler();
}

void Exo::loadScrobbler() {
    if(scrobbler)
        return;
    scrobbler = new Scrobbler(this);
    connect(player, SIGNAL(trackChanged(QString, QString, int)),
            scrobbler, SLOT(init(QString, QString, int)));
    connect(player, SIGNAL(trackListened(QString, QString, QString, int)),
            scrobbler, SLOT(submit(QString, QString, QString, int)));
    emit scrobblerLoaded(true);
}

void Exo::scrobblerToggle(bool checked) {
    settingsObject->setValue("scrobbler/enabled", checked);
    emit scrobblerLoaded(false);
    if(checked && !scrobbler)
        configureScrobbler();
    else if(scrobbler)
        scrobbler->deleteLater();
    else
        qCritical("Exo::scrobblerToggle - unexpected condition");
}
#endif // BUILD_LASTFM
