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
#include "core/cmusinterface.h"
#include "core/mocinterface.h"
#include "core/mocinterfacenative.h"
#include "core/spotifyinterface.h"
#include "dbus/dbus.h"
#include "gui/trayicon.h"

#ifdef BUILD_LASTFM
#include "core/consoleauth.h"
#include "lastfm/scrobbler.h"
#endif // BUILD_LASTFM

#define CAST_PI static_cast<std::unique_ptr<PlayerInterface>>


void initObjects(PlayerInterface* player, const QSettings& settings) {
    if(!QString(QLatin1String(qgetenv("DISPLAY"))).isEmpty()) {
        DBus::init(player);
    }
#ifdef BUILD_LASTFM
    if(settings.value(QStringLiteral("scrobbler/enabled")).toBool()) {
        new Scrobbler(player);
    }
    if(settings.value(QStringLiteral("scrobbler/streams")).toBool()) {
        player->enableStreamsScrobbling(true);
    }
#endif // BUILD_LASTFM
    if(QString(QLatin1String(qgetenv("DISPLAY"))).isEmpty()
        && !settings.value(QStringLiteral("scrobbler/enabled")).toBool()) {
        qWarning("Both scrobbler and DBus interface were disabled. "
                 "Please remember that running the app in background as such is pointless.");
    }
}

int main(int argc, char *argv[]) {
    bool useGui = true;
    bool useSpotify = false;
    bool forceReauth = false;
    bool useCmus = false;
    bool useMocNative = false;
    QStringList inputFiles{};

    if(argc > 1) {
        QByteArray arg = argv[1];
        if(arg == QByteArray("-h") || arg == QByteArray("--help")) {
            QTextStream out(stdout);
            out << "Usage: exo [-h] [-b] [-c] [-f] [-n] [-s]\nSee also `man exo`\n";
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
                }
                else if(arg == QByteArray("-n") || arg == QByteArray("--use-native")) {
                    useMocNative = true;
                } else {
                    if(QFile::exists(arg)) {
                        inputFiles.append(arg);
                    }
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
    std::unique_ptr<PlayerInterface> player = nullptr;

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
        player = std::make_unique<SpotifyInterface>();
        Q_UNUSED(player);
        QSettings settings;
        new Scrobbler(player.get());
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
        player = useCmus ? std::make_unique<CmusInterface>()
                 : useMocNative ? std::make_unique<MocInterfaceNative>()
                                : CAST_PI(std::make_unique<MocInterface>());
        if(!instance.isUnique()) {
            if(inputFiles.size() == 1) {
                player->openUri(inputFiles.at(0));
            } else if(inputFiles.size() > 1) {
                player->appendFile(inputFiles);
            } else {
                qWarning("Application is already running. Showing player window.");
                player->showPlayer();
            }
            return 0;
        }
        QSettings settings;
        initObjects(player.get(), settings);
        Q_INIT_RESOURCE(exo);
        TrayIcon trayIcon;
        Q_UNUSED(trayIcon);
        if(inputFiles.size() == 1) {
            player->openUri(inputFiles.at(0));
        } else if(inputFiles.size() > 1) {
            player->appendFile(inputFiles);
        }
        app.exec();
        if(settings.value(QStringLiteral("player/quit")).toBool()) {
            player->quit();
        }
        player->shutdown();
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
        player = useCmus ? std::make_unique<CmusInterface>()
                 : useMocNative ? std::make_unique<MocInterfaceNative>()
                                : CAST_PI(std::make_unique<MocInterface>());
        QSettings settings;
        initObjects(player.get(), settings);
        app.exec();
        if(settings.value(QStringLiteral("player/quit")).toBool()) {
            player->quit();
        }
        player->shutdown();
    }

    return 0;
}
