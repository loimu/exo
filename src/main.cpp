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

#include <QByteArray>
#include <QNetworkProxyFactory>
#include <QSettings>
#include "exo.h"

int main(int argc, char *argv[]) {
    Q_INIT_RESOURCE(exo);
    for(int i=1; i<argc; i++) {
        QByteArray arg = argv[i];
        if(arg == "-d" || arg == "--daemonize")
            Exo::useGui = false;
        if(arg == "-n" || arg == "--no-dbus")
            Exo::useDBus = false;
    }
    QCoreApplication::setOrganizationName("exo");
    QCoreApplication::setApplicationName("eXo");
    QCoreApplication::setApplicationVersion("0.5");
    QNetworkProxyFactory::setUseSystemConfiguration(true);
    QSettings settings;
    Exo::settings = &settings;
    Exo app(argc, argv, Exo::useGui);
    return app.exec();
}
