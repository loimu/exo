/* ========================================================================
*    Copyright (C) 2013-2024 Blaze <blaze@vivaldi.net>
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

#include <QObject>
#include <QDBusConnection>
#include <QDBusInterface>

#include "trayicon.h"
//MPRISv2
#include "rootobject.h"
#include "playerobject.h"

#include "playerinterface.h"

#include "dbus.h"

#define QSL QStringLiteral


class DBusAdaptor : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "local.exo_player")

public:
    explicit DBusAdaptor(QObject* parent = nullptr) : QObject(parent) {}

public Q_SLOTS:
    void showLyricsWindow() {
        if(TrayIcon::self()) {
            TrayIcon::self()->showLyricsWindow(-1);
        }
    }
};


void DBus::init(QObject* parent) {
    new RootObject(parent);
    new PlayerObject(parent);
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.registerObject(
        QSL("/exo"),new DBusAdaptor(parent),QDBusConnection::ExportAllContents);
    if(!connection.registerService(QSL("local.exo_player"))) {
        qWarning("DBus: service registration failed");
    }
    connection.registerObject(QSL("/org/mpris/MediaPlayer2"), parent);
    if(!connection.registerService(QSL("org.mpris.MediaPlayer2.exo"))) {
        qWarning("DBus: MPRISv2 service registration failed");
    }

    // Show a system notification through a session DBus object
    QObject::connect(PLAYER, &PlayerInterface::newTrack,
                     parent, [connection] (const QString& cover) {
        const PTrack track = PLAYER->getTrack();
        const QString title = track.isStream ? track.title
                                             : QString("%1 (%2)").arg(
                                                   track.title, track.totalTime);
        QDBusInterface notify{QSL("org.freedesktop.Notifications"),
                              QSL("/org/freedesktop/Notifications"),
                              QSL("org.freedesktop.Notifications"), connection};
        notify.callWithArgumentList(QDBus::NoBlock, QStringLiteral("Notify"),
                                    // signature: s u s s s as a{sv} i
                                    QList<QVariant>{
                                        PLAYER->id(), // app_name
                                        quint32(0),   // replaces_id
                                        cover,        // app_icon
                                        track.artist, // summary
                                        title,        // body
                                        //  actions, hints, timeout
                                        QStringList{}, QVariantMap{}, 10000
                                    });
    });
}

#include "dbus.moc"
