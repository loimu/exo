/* ========================================================================
*    Copyright (C) 2013-2016 Blaze <blaze@vivaldi.net>
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

#ifdef BUILD_DBUS
#include "dbus/dbus.h"
#endif // BUILD_DBUS
#ifdef BUILD_LASTFM
#include "core/consoleauth.h"
#include "lastfm/scrobbler.h"
#endif // BUILD_LASTFM
#ifdef USE_CMUS
#include "cmusinterface.h"
#else // USE_CMUS
#include "mocplayerinterface.h"
#endif // USE_CMUS

#include "gui/trayicon.h"
#include "playerinterface.h"
#include "exo.h"

bool Exo::useDBus = true;
bool Exo::useGui = true;
Exo* Exo::instance = nullptr;
QSettings* Exo::settings = nullptr;

Exo::Exo(int &argc, char **argv, bool useGui) : QApplication(argc, argv, useGui)
{
    settings = new QSettings();
    instance = this;
    setQuitOnLastWindowClosed(false);

#ifdef USE_CMUS
    player = new CmusInterface(this);
#else // USE_CMUS
    player = new MOCPlayerInterface(this);
#endif // USE_CMUS

#ifdef BUILD_DBUS
    if(useDBus)
        new DBus(this);
#endif // BUILD_DBUS

#ifdef BUILD_LASTFM
    if(settings->value("scrobbler/enabled").toBool())
        loadScrobbler(true);
    if(!useGui && !settings->value("scrobbler/sessionkey").toBool())
        new ConsoleAuth();
#endif // BUILD_LASTFM

    if(useGui && QSystemTrayIcon::isSystemTrayAvailable()) {
        trayIcon = new TrayIcon(this);
        trayIcon->hide();
    }
}

Exo::~Exo() {
    if(settings->value("player/quit").toBool())
       player->quit();
    if(trayIcon)
        trayIcon->deleteLater();
}

void Exo::showLyricsWindow() {
    trayIcon->showLyricsWindow();
}

#ifdef BUILD_LASTFM
void Exo::loadScrobbler(bool checked) {
    if(!scrobbler && checked) {
        scrobbler = new Scrobbler(this);
        connect(player, SIGNAL(trackChanged(QString, QString, int)),
                scrobbler, SLOT(init(QString, QString, int)));
        connect(player, SIGNAL(trackListened(QString, QString, QString, int)),
                scrobbler, SLOT(submit(QString, QString, QString, int)));
    } else if(scrobbler && !checked)
        scrobbler->deleteLater();
}
#endif // BUILD_LASTFM
