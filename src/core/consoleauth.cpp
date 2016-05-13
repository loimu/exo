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

#include <QTextStream>

#include "lastfm/scrobblerauth.h"
#include "consoleauth.h"

ConsoleAuth::ConsoleAuth(QObject *parent) : QObject(parent)
{
    scrobblerAuth = new ScrobblerAuth(this);
    connect(scrobblerAuth, SIGNAL(failed(QString)), SLOT(authFail(QString)));
    connect(scrobblerAuth, SIGNAL(configured()), SLOT(authSuccess()));
    auth();
}

void ConsoleAuth::auth() {
    QTextStream so(stdout);
    QTextStream si(stdin);
    so << tr("Last.fm authentication");
    so << tr("Login:");
    QString login = si.readLine();
    so << tr("Password:");
    QString password = si.readLine();
    scrobblerAuth->auth(login, password);
}

void ConsoleAuth::authFail(const QString& errmsg) {
    QTextStream so(stdout);
    so << errmsg;
    so << tr("Try again? (y/n)");
    QTextStream si(stdin);
    QString input = si.readLine();
    if(input != tr("y") || input != tr("Y")) {
        this->deleteLater();
        return;
    }
    auth();
}

void ConsoleAuth::authSuccess() {
    QTextStream so(stdout);
    so << tr("Authentication succesful!");
    this->deleteLater();
}
