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

#include "config.h"

#include <unistd.h>

#include <QNetworkProxyFactory>
#include <QApplication>
#include <QSettings>
#include <QTranslator>
#include <QFile>

#include "core/singleinstance.h"
#ifdef BUILD_DBUS
  #include "dbus/dbus.h"
#endif // BUILD_DBUS
#ifdef BUILD_LASTFM
  #include "core/consoleauth.h"
  #include "lastfm/scrobbler.h"
#endif // BUILD_LASTFM
#include "core/cmusinterface.h"
#include "core/mocinterface.h"
#include "core/spotifyinterface.h"
#include "gui/trayicon.h"

#define INIT_PLAYER if(useCmus) player = new CmusInterface(&app);\
  else player = new MocInterface(&app);


void initObjects(QCoreApplication& app, const QSettings& settings) {
  #ifdef BUILD_DBUS
    if(!QString(QLatin1String(qgetenv("DISPLAY"))).isEmpty())
        DBus::init(&app);
  #endif // BUILD_DBUS
  #ifdef BUILD_LASTFM
    if(settings.value(QStringLiteral("scrobbler/enabled")).toBool())
        new Scrobbler(&app);
  #endif // BUILD_LASTFM
}

int main(int argc, char *argv[]) {
    bool useGui = true;
    bool useSpotify = false;
    bool forceReauth = false;
    bool useCmus = false;
    QString inputFile{};

    if(argc > 1) {
        QByteArray arg = argv[1];
        if(arg == QByteArray("-h") || arg == QByteArray("--help")) {
            QTextStream out(stdout);
            out << "Usage: exo [-h] [-b] [-c] [-f] [-s]\nSee also `man exo`\n";
            return 0;
        }
        else if(arg == QByteArray("-d") || arg == QByteArray("-b")
                || arg == QByteArray("--background")) {
            useGui = false;
            if(::fork() != 0) return 0;
            if(::fork() != 0) return 0;
            else qDebug("Running in the background");
        }
        else if(arg == QByteArray("-f") || arg == QByteArray("--force-reauth")) {
            useGui = false;
            forceReauth = true;
        }
#ifdef BUILD_LASTFM
        else if(arg == QByteArray("-s") || arg == QByteArray("--use-spotify")) {
            useSpotify = true;
            if(::fork() != 0) return 0;
            if(::fork() != 0) return 0;
            else qDebug("Running in the background as Spotify adaptor");
        }
#endif // BUILD_LASTFM
        else {
            // non-exclusive input options are being processed in a cycle
            for(int i = 1; i < argc; i++) {
                QByteArray arg = argv[i];
                if(arg == QByteArray("-c") || arg == QByteArray("--use-cmus")) {
                    useCmus = true;
                } else {
                    inputFile = arg;
                }
            }
        }
    }

    QCoreApplication::setOrganizationName(QLatin1String("exo"));
    QCoreApplication::setApplicationName(QLatin1String("eXo"));
    QCoreApplication::setApplicationVersion(QLatin1String(EXO_VERSION));
    QNetworkProxyFactory::setUseSystemConfiguration(true);
    SingleInstance instance(useSpotify ? QLatin1String("eXo_spotify") : QLatin1String("eXo"));
    QTranslator translator;
    PlayerInterface* player = nullptr;

#ifdef BUILD_LASTFM
    if(useSpotify) {
        QCoreApplication app(argc, argv);
        if(QString(QLatin1String(qgetenv("DISPLAY"))).isEmpty()) {
            qWarning("No graphical session is detected");
            return 1;
        }
        if(!instance.isUnique()) {
            qWarning("Application is already running");
            return 1;
        }
        player = new SpotifyInterface(&app);
        Q_UNUSED(player);
        QSettings settings;
        new Scrobbler(&app);
        app.exec();
    }
#endif // BUILD_LASTFM

    if(useGui) {
        /* graphical application */
        QApplication app(argc, argv);
        app.setQuitOnLastWindowClosed(false);
        bool res = translator.load(QApplication::applicationDirPath() +
                                   QLatin1String("/../share/exo/translations/") +
                                   QLocale::system().name() + QLatin1String(".qm"));
        Q_UNUSED(res);
        app.installTranslator(&translator);
        if(!instance.isUnique()) {
            INIT_PLAYER
            player->showPlayer();
            if(!inputFile.isEmpty() && QFile::exists(inputFile)) {
                player->openUri(inputFile);
            }
            return 0;
        }
        INIT_PLAYER
        QSettings settings;
        initObjects(app, settings);
        Q_INIT_RESOURCE(exo);
        TrayIcon trayIcon;
        Q_UNUSED(trayIcon);
        if(!inputFile.isEmpty() && QFile::exists(inputFile)) {
            player->openUri(inputFile);
        }
        app.exec();
        player->shutdown();
        if(settings.value(QStringLiteral("player/quit")).toBool())
            player->quit();
    } else {
        /* console application */
        QCoreApplication app(argc, argv);
        if(!instance.isUnique()) {
            qWarning("Application is already running");
            return 1;
        }
        bool res = translator.load(QApplication::applicationDirPath() +
                                   QLatin1String("/../share/exo/translations/") +
                                   QLocale::system().name() + QLatin1String(".qm"));
        Q_UNUSED(res);
        app.installTranslator(&translator);
        if(forceReauth) {
#ifdef BUILD_LASTFM
            new ConsoleAuth(&app);
            return app.exec();
#else
            qWarning("Scrobbler has been disabled during the build time");
            return 1;
#endif // BUILD_LASTFM
        }
        INIT_PLAYER
        QSettings settings;
        initObjects(app, settings);
        app.exec();
        player->shutdown();
        if(settings.value(QStringLiteral("player/quit")).toBool())
            player->quit();
    }

    return 0;
}
