/* ========================================================================
*    Copyright (C) 2013-2017 Blaze <blaze@vivaldi.net>
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

#include <unistd.h>

#include <QByteArray>
#include <QNetworkProxyFactory>
#include <QSettings>
#include <QApplication>
#include <QMessageBox>

#include "core/singleinstance.h"

#ifdef BUILD_DBUS
  #include "dbus/dbus.h"
#endif // BUILD_DBUS

#ifdef BUILD_LASTFM
  #include "core/consoleauth.h"
  #include "lastfm/scrobbler.h"
#endif // BUILD_LASTFM

#ifdef USE_CMUS
  #include "core/cmusinterface.h"
#endif // USE_CMUS
#include "core/mocinterface.h"
#include "gui/trayicon.h"


int main(int argc, char *argv[]) {
    bool useGui = true;
    bool forceReauth = false;
    for(int i=1; i<argc; i++) {
        QByteArray arg = argv[i];
        if(arg == QByteArray("-d") || arg == QByteArray("-b")
                || arg == QByteArray("--background")) {
            useGui = false;
            if(fork() == 0)
                qWarning("Running in the background succeeded");
            else
                return 0; // exiting the parent process or error condition
        }
        if(arg == QByteArray("-f") || arg == QByteArray("--force-reauth")) {
            useGui = false;
            forceReauth = true;
        }
    }

    QCoreApplication::setOrganizationName(QLatin1String("exo"));
    QCoreApplication::setApplicationName(QLatin1String("eXo"));
    QCoreApplication::setApplicationVersion(QLatin1String("0.7"));
    QNetworkProxyFactory::setUseSystemConfiguration(true);
    QApplication app(argc, argv, useGui);
    app.setQuitOnLastWindowClosed(false);

    SingleInstance inst;
    if(!inst.isUnique()) {
        qWarning("Application is already running");
        if(useGui)
            QMessageBox::critical(
                        nullptr, app.applicationName(),
                        QObject::tr("Application is already running"));
        return 1;
    }

    if(forceReauth) {
#ifdef BUILD_LASTFM
        new ConsoleAuth(&app);
        return app.exec();
#else
        qWarning("Scrobbler has been disabled with a build flag");
        return 1;
#endif // BUILD_LASTFM
    }

#ifdef USE_CMUS
    CmusInterface cmusInterface(&app);
#else // USE_CMUS
    MocInterface mocInterface(&app);
#endif // USE_CMUS

#ifdef BUILD_DBUS
    if(!QString(QLatin1String(getenv("DISPLAY"))).isEmpty())
        new DBus(&app);
#endif // BUILD_DBUS

#ifdef BUILD_LASTFM
    QSettings settings;
    if(settings.value(QLatin1String("scrobbler/enabled")).toBool())
        new Scrobbler(&app);
#endif // BUILD_LASTFM

    if(!useGui || !QSystemTrayIcon::isSystemTrayAvailable())
        return app.exec();
    Q_INIT_RESOURCE(exo);
    TrayIcon trayIcon;
    return app.exec();
}
