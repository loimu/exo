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

#include <QtDBus>

#include "dbus.h"
#include "exoobject.h"
//MPRISv2
#include "rootobject.h"
#include "playerobject.h"

DBus::DBus(QObject *parent) : QObject(parent)
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.registerObject("/Exo", new ExoObject(this), QDBusConnection::ExportAllContents);
    bool registered = connection.registerService("tk.loimu.exo");
    if(!registered) {
        qFatal("Only one instance of application is allowed.\n"
               "Or try to use the \"-n\" option to start the app with no DBus\n"
               "if you're on terminal.");
    }
    new RootObject(this);
    new PlayerObject(this);
    connection.registerObject("/org/mpris/MediaPlayer2", this);
    connection.registerService("org.mpris.MediaPlayer2.exo");
}

DBus::~DBus()
{
    QDBusConnection::sessionBus().unregisterService("tk.loimu.exo");
    QDBusConnection::sessionBus().unregisterService("org.mpris.MediaPlayer2.exo");
}
