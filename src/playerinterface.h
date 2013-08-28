/* ========================================================================
*    Copyright (C) 2013 Blaze <blaze@jabster.pl>
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
#include <QStringList>

class PlayerInterface : public QObject
{
    Q_OBJECT

    void runServer();

    static PlayerInterface* m_instance;
    bool m_listened;

public:
    PlayerInterface(QObject *parent = 0);
    ~PlayerInterface();

    bool isServerRunning();
    void openWindow();

    QString m_title;
    QStringList m_list;

signals:
    void trackListened();
    void trackChanged();
    void updateStatus();

public slots:
    void play();
    void pause();
    void prev();
    void next();
    void stop();
    void quit();

private slots:
    void update();
};

#endif // PLAYERINTERFACE_H
