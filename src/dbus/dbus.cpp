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
#include <QDBusReply>
#include <QDateTime>

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

    auto newTrackHandler = [] (const QString& cover, bool paused = false) {
        const PTrack track = PLAYER->getTrack();
        const QString artist = track.isStream ? track.artist
                                              : QString(QLatin1String("%1 - %2")).arg(
                                                    track.artist, track.album);
        const QString title = track.isStream && !paused
                                  ? track.title : QString(QLatin1String("%1 (%2)")).arg(
                                        track.title,
                                        paused ? QObject::tr("Paused") : track.totalTime);
        notify(PLAYER->id(), cover, title, artist);
    };
    QObject::connect(PLAYER, &PlayerInterface::newTrack, parent, newTrackHandler);
    QObject::connect(PLAYER, &PlayerInterface::paused, parent, newTrackHandler);
}

void DBus::notify(const QString& appName, const QString& icon,
                  const QString& summary, const QString& body) {
    const int timeoutSeconds = 10;
    static quint32 replacesId = 0;
    static QDateTime notificationTime = QDateTime::currentDateTime();
    QDateTime currentTime =QDateTime::currentDateTime();
    if(notificationTime.addSecs(timeoutSeconds) < currentTime) {
        replacesId = 0;
    }
    notificationTime = currentTime;
    // Show a system notification through the session DBus object
    QDBusInterface notify{QSL("org.freedesktop.Notifications"),
                          QSL("/org/freedesktop/Notifications"),
                          QSL("org.freedesktop.Notifications")};
    QDBusReply<quint32> reply = notify.call(
        QStringLiteral("Notify"),
        // signature: s u s s s as a{sv} i
        appName,    // app_name
        replacesId, // replaces_id
        icon,       // app_icon
        summary,    // summary
        body,       // body
        //  actions, hints, timeout
        QStringList{}, QVariantMap{}, timeoutSeconds * 1000
        );
    if(reply.isValid()) {
        replacesId = reply.value();
    }
}

#include "dbus.moc"
