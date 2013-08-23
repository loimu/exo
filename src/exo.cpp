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

#include "exo.h"
#include "scrobbler.h"
#include "trayicon.h"
#include "playerinterface.h"
#include "scrobblersettings.h"

Exo::Exo(int &argc, char **argv) : QApplication(argc, argv)
{
    QSettings* settings = new QSettings("latitude52", "eXo");
    bool configured = settings->value("scrobbler/configured").toBool();
    if(!configured) {
        ScrobblerSettings* settingsDialog = new ScrobblerSettings(settings);
        settingsDialog->show();
    }

    PlayerInterface* player = new PlayerInterface(this);

    TrayIcon *trayicon = new TrayIcon(player);
    trayicon->hide();

    bool enabled = settings->value("scrobbler/enabled").toBool();
    if(enabled) {
        Scrobbler* scrobbler = new Scrobbler(this, settings, player);
    }
}
