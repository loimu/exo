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
#include "core/cmusinterface.h"
#else // USE_CMUS
#include "core/mocinterface.h"
#endif // USE_CMUS
#include "gui/trayicon.h"
#include "exo.h"

Exo* Exo::object = nullptr;
bool Exo::reauth = false;

Exo::Exo(int &argc, char **argv, bool useGui) : QApplication(argc, argv, useGui)
{
    object = this;
    setQuitOnLastWindowClosed(false);

#ifdef USE_CMUS
    new CmusInterface(this);
#else // USE_CMUS
    new MocInterface(this);
#endif // USE_CMUS

#ifdef BUILD_DBUS
    if(!QString(getenv("TERM")).startsWith("linux"))
        new DBus(this);
#endif // BUILD_DBUS

#ifdef BUILD_LASTFM
    QSettings settings;
    if(settings.value("scrobbler/enabled").toBool())
        loadScrobbler(true);
    if(!useGui && (reauth || !settings.value("scrobbler/sessionkey").toBool()))
        new ConsoleAuth(this);
#endif // BUILD_LASTFM

    if(useGui && QSystemTrayIcon::isSystemTrayAvailable()) {
        TrayIcon* trayIcon = new TrayIcon(this);
        trayIcon->hide();
    }
}

Exo::~Exo() {
    QSettings settings;
    if(settings.value("player/quit").toBool())
       PlayerInterface::self()->quit();
    if(TrayIcon::self())
        TrayIcon::self()->deleteLater();
}

Exo* Exo::self() {
    return object;
}

void Exo::forceReauth() {
    reauth = true;
}

#ifdef BUILD_LASTFM
void Exo::loadScrobbler(bool checked) {
    if(!scrobbler && checked) {
        scrobbler = new Scrobbler(this);
        PlayerInterface* player = PlayerInterface::self();
        connect(player, SIGNAL(trackChanged(QString, QString, int)),
                scrobbler, SLOT(init(QString, QString, int)));
        connect(player, SIGNAL(trackListened(QString, QString, QString, int)),
                scrobbler, SLOT(submit(QString, QString, QString, int)));
    } else if(scrobbler && !checked)
        scrobbler->deleteLater();
}
#endif // BUILD_LASTFM
