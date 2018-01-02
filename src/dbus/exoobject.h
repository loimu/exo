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

#ifndef EXOOBJECT_H
#define EXOOBJECT_H

#include <QObject>

class ExoObject : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "local.exo_player")

public:
    explicit ExoObject(QObject* parent = nullptr);

public slots:
    void showLyricsWindow();
    void quitApplication();
    void bookmarkCurrent();
};

#endif // EXOOBJECT_H
