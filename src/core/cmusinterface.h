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

#ifndef CMUSINTERFACE_H
#define CMUSINTERFACE_H

#include "playerinterface.h"

class QProcess;

class CmusInterface : public PlayerInterface
{
    QProcess* cmus;
    QString cli;
    void runPlayer();
    QString find(const QString&, const QString&);
    PState updateInfo();
    void timerEvent(QTimerEvent* event);

public:
    explicit CmusInterface(QObject* parent = nullptr);
    ~CmusInterface();
    QString id() const;
    void play();
    void pause();
    void playPause();
    void prev();
    void next();
    void stop();
    void quit();
    void jump(int);
    void seek(int);
    void volume(int);
    void changeVolume(int);
    void showPlayer();
    void openUri(const QString&);
    void appendFile(const QStringList&);
    void clearPlaylist();
};

#endif // CMUSINTERFACE_H
