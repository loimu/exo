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

#ifndef PLAYERINTERFACE_H
#define PLAYERINTERFACE_H

#include <QObject>

class PlayerInterface : public QObject
{
    Q_OBJECT

    static PlayerInterface* object;
    QString execute(QString, QStringList);
    void sendOption(QString);
    void runServer();
    bool isServerRunning();
    QString artistString;
    QString titleString;

public:
    PlayerInterface(QObject *parent = 0);
    static PlayerInterface* instance();
    QString artist();
    QString title();

signals:
    void trackListened(QString, QString, QString, int);
    void trackChanged(QString, QString, int);
    void updateStatus(QString, QString, QString, QString);

public slots:
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
    void openWindow();
    void appendFile(QString);

private slots:
    void update();
};

#endif // PLAYERINTERFACE_H
