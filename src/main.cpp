/* ========================================================================
*    Copyright (C) 2013-2018 Blaze <blaze@vivaldi.net>
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

#ifdef BUILD_TRANSLATIONS
  #include <QTranslator>
#endif // BUILD_TRANSLATIONS

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
    if(argc > 1) {
        QByteArray arg = argv[1];
        if(arg == QByteArray("-h") || arg == QByteArray("--help")) {
            qInfo("Usage: exo [-h] [-b] [-f]\nSee also `man exo`");
            return 0;
        }
        else if(arg == QByteArray("-d") || arg == QByteArray("-b")
                || arg == QByteArray("--background")) {
            useGui = false;
            if(fork() != 0) return 0;
            if(fork() != 0) return 0;
            else qDebug("Running in the background");
        }
        else if(arg == QByteArray("-f") || arg == QByteArray("--force-reauth")){
            useGui = false;
            forceReauth = true;
        }
    }

    QCoreApplication::setOrganizationName(QLatin1String("exo"));
    QCoreApplication::setApplicationName(QLatin1String("eXo"));
    QCoreApplication::setApplicationVersion(QLatin1String(EXO_VERSION));
    QNetworkProxyFactory::setUseSystemConfiguration(true);
    SingleInstance instance;
    int result = 0;
#ifdef BUILD_TRANSLATIONS
    QTranslator translator;
#endif // BUILD_TRANSLATIONS

    if(useGui) {
        /* graphical application */
        QApplication app(argc, argv);
        app.setQuitOnLastWindowClosed(false);
#ifdef BUILD_TRANSLATIONS
        translator.load(QApplication::applicationDirPath() +
                        QLatin1String("/../share/exo/translations/") +
                        QLocale::system().name() + QLatin1String(".qm"));
        app.installTranslator(&translator);
#endif // BUILD_TRANSLATIONS
        if(!instance.isUnique()) {
            QMessageBox::critical(
                        nullptr, app.applicationName(),
                        QObject::tr("Application is already running"));
            return 1;
        }

#ifdef USE_CMUS
        CmusInterface player(&app);
#else // USE_CMUS
        MocInterface player(&app);
#endif // USE_CMUS

#ifdef BUILD_DBUS
        new DBus(&app);
#endif // BUILD_DBUS

        QSettings settings;

#ifdef BUILD_LASTFM
        if(settings.value(QStringLiteral("scrobbler/enabled")).toBool())
            new Scrobbler(&app);
#endif // BUILD_LASTFM

        if(!QSystemTrayIcon::isSystemTrayAvailable())
            qWarning("System tray is not available. Running with no tray");
        Q_INIT_RESOURCE(exo);
        TrayIcon trayIcon;
        Q_UNUSED(trayIcon);

        result = app.exec();
        if(settings.value(QStringLiteral("player/quit")).toBool())
            player.quit();
        /* end of graphical application */
    } else {
        /* console application */
        QCoreApplication app(argc, argv);
        if(!instance.isUnique()) {
            qWarning("Application is already running");
            return 1;
        }

#ifdef BUILD_TRANSLATIONS
        translator.load(QApplication::applicationDirPath() +
                        QLatin1String("/../share/exo/translations/") +
                        QLocale::system().name() + QLatin1String(".qm"));
        app.installTranslator(&translator);
#endif // BUILD_TRANSLATIONS
        if(forceReauth) {
#ifdef BUILD_LASTFM
            new ConsoleAuth(&app);
            return app.exec();
#else
            qWarning("Scrobbler has been disabled during the build time");
            return 1;
#endif // BUILD_LASTFM
        }

#ifdef USE_CMUS
        CmusInterface player(&app);
#else // USE_CMUS
        MocInterface player(&app);
#endif // USE_CMUS

#ifdef BUILD_DBUS
        if(!QString(QLatin1String(getenv("DISPLAY"))).isEmpty())
            new DBus(&app);
#endif // BUILD_DBUS

        QSettings settings;

#ifdef BUILD_LASTFM
        if(settings.value(QStringLiteral("scrobbler/enabled")).toBool())
            new Scrobbler(&app);
#endif // BUILD_LASTFM

        result = app.exec();
        if(settings.value(QStringLiteral("player/quit")).toBool())
            player.quit();
        /* end of console application */
    }
    return result;
}
