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

#ifndef MOCPLAYERINTERFACE_H
#define MOCPLAYERINTERFACE_H

class MOCPlayerInterface : public PlayerInterface
{
    void sendOption(QString);
    void runServer();
    bool isServerRunning();

public:
    explicit MOCPlayerInterface(QObject *parent = 0);

signals:

public slots:
    QString id();
    void play();
    void pause();
    void prev();
    void next();
    void stop();
    void quit();
    void volu();
    void vold();
    void rewd();
    void frwd();
    void showPlayer();
    void appendFile(QString);

protected slots:
    void update();
    void getInfo();

};

#endif // MOCPLAYERINTERFACE_H
