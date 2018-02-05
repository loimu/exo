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

#ifndef MOCINTERFACE_H
#define MOCINTERFACE_H

#include "playerinterface.h"

class QProcess;

class MocInterface : public PlayerInterface
{
    QProcess* moc;
    QString player;
    bool runServer();
    PIState updateInfo();
    void timerEvent(QTimerEvent* event);

public:
    explicit MocInterface(QObject* parent = nullptr);
    ~MocInterface();
    QString id();
    bool play();
    bool pause();
    bool playPause();
    bool prev();
    bool next();
    bool stop();
    bool quit();
    bool jump(int);
    bool seek(int);
    bool volume(int);
    bool changeVolume(int);
    void showPlayer();
    bool openUri(const QString&);
    bool appendFile(const QStringList&);
};

#endif // MOCINTERFACE_H
