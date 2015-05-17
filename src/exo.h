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

#ifndef EXO_H
#define EXO_H

#include "config.h"

#if QT_VERSION >= 0x050000
    #include <QtWidgets>
#endif

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
    explicit Exo(int &argc, char **argv, bool);
    ~Exo();
    static Exo* app();
    QSettings* settings();

private slots:
#ifdef BUILD_LASTFM
    void configureScrobbler();
    void loadScrobbler();
    void scrobblerToggle(bool);
#endif // BUILD_LASTFM

signals:
    void lyricsWindow();
    void scrobblerLoaded(bool);

public slots:
    void showLyricsWindow();
};

#endif // EXO_H
