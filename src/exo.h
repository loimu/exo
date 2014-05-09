/* ========================================================================
*    Copyright (C) 2013-2014 Blaze <blaze@jabster.pl>
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

#ifndef EXO_H
#define EXO_H

#include <QApplication>
#include <QPointer>

class Scrobbler;
class PlayerInterface;
class QSettings;

class Exo : public QApplication
{
    Q_OBJECT

    QPointer<Scrobbler> scrobbler;
    PlayerInterface* player;
    QSettings* settingsObject;
    void init(bool);

public:
    explicit Exo(int &argc, char **argv, bool, QString, QString);
    static Exo* app();
    QSettings* settings();

private slots:
    void configureScrobbler();
    void loadScrobbler();
    void unloadScrobbler();
    void enableScrobbler();
};

#endif // EXO_H
