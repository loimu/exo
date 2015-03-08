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

#ifndef PLAYERINTERFACE_H
#define PLAYERINTERFACE_H

#include <QObject>

struct Track
{
    QString state, artist, song, album, file, totalTime, currTime, title;
    int totalSec, currSec;
};

class PlayerInterface : public QObject
{
    Q_OBJECT

    static PlayerInterface* object;

protected:
    Track track;
    void startTimer(int);
    QString execute(QString, QStringList);
    void scrobbler();
    QString cover();

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
    virtual void play() = 0;
    virtual void pause()= 0;
    virtual void prev() = 0;
    virtual void next() = 0;
    virtual void stop() = 0;
    virtual void quit() = 0;
    virtual void volu() = 0;
    virtual void vold() = 0;
    virtual void rewd() = 0;
    virtual void frwd() = 0;
    virtual void openWindow() = 0;
    virtual void appendFile(QString) = 0;

protected slots:
    virtual void update() = 0;
    virtual void getInfo() = 0;
};

#endif // PLAYERINTERFACE_H
