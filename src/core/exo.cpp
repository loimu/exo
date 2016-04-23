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

#ifdef BUILD_DBUS
#include "dbus/dbus.h"
#endif // BUILD_DBUS
#ifdef BUILD_LASTFM
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
Exo* Exo::instance = 0;
QSettings* Exo::settings = 0;

Exo::Exo(int &argc, char **argv, bool useGui) : QApplication(argc, argv, useGui)
{
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
    enableScrobbler(settings->value("scrobbler/enabled").toBool() &&
            settings->value("scrobbler/sessionkey").toBool());
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
void Exo::enableScrobbler(bool checked) {
    if(scrobbler && checked) {
        qWarning("scrobbler already loaded");
        return;
    }
    if(!scrobbler && !checked) {
        qWarning("scrobbler already unloaded");
        return;
    }
    settings->setValue("scrobbler/enabled", checked);
    if(checked) {
        scrobbler = new Scrobbler(this);
        connect(player, SIGNAL(trackChanged(QString, QString, int)),
                scrobbler, SLOT(init(QString, QString, int)));
        connect(player, SIGNAL(trackListened(QString, QString, QString, int)),
                scrobbler, SLOT(submit(QString, QString, QString, int)));
    } else
        scrobbler->deleteLater();
}
#endif // BUILD_LASTFM
