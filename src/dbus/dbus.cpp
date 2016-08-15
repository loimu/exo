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

#include <QtDBus>

#include "dbus.h"
#include "dbus/exoobject.h"
//MPRISv2
#include "dbus/rootobject.h"
#include "dbus/playerobject.h"

DBus::DBus(QObject *parent) : QObject(parent)
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.registerObject(QLatin1String("/Exo"), new ExoObject(this),
                              QDBusConnection::ExportAllContents);
    bool registered = connection.registerService(QLatin1String("tk.loimu.exo"));
    if(!registered)
        qFatal("Only one instance of application is allowed.\n");
    new RootObject(this);
    new PlayerObject(this);
    connection.registerObject(QLatin1String("/org/mpris/MediaPlayer2"), this);
    connection.registerService(QLatin1String("org.mpris.MediaPlayer2.exo"));
}

DBus::~DBus()
{
    QDBusConnection::sessionBus().unregisterService(
                QLatin1String("tk.loimu.exo"));
    QDBusConnection::sessionBus().unregisterService(
                QLatin1String("org.mpris.MediaPlayer2.exo"));
}
