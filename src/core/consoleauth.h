/* ========================================================================
*    Copyright (C) 2013-2023 Blaze <blaze@vivaldi.net>
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

#ifndef CONSOLEAUTH_H
#define CONSOLEAUTH_H

#include <QObject>

class ScrobblerAuth;


class ConsoleAuth : public QObject
{
    Q_OBJECT

    ScrobblerAuth* scrobblerAuth;
    void auth();
    void authFail(const QString& errmsg);
    void authSuccess();

public:
    explicit ConsoleAuth(QObject *parent = nullptr);
};

#endif // CONSOLEAUTH_H
