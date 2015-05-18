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
    QString getOutput(QString, QStringList);
    bool execute(QString, QStringList);
    void scrobble();
    QString cover();

public:
    PlayerInterface(QObject *parent = 0);
    static PlayerInterface* instance();
    QString artist();
    QString title();
    QString album();
    QString artwork();
    QString url();
    quint64 length();
    quint64 position();

signals:
    void trackListened(QString, QString, QString, int);
    void trackChanged(QString, QString, int);
    void updateStatus(QString, QString, QString, QString);
    void statusChanged(QString);

public slots:
    virtual QString id() = 0;
    virtual bool play() = 0;
    virtual bool pause()= 0;
    virtual bool playPause() = 0;
    virtual bool prev() = 0;
    virtual bool next() = 0;
    virtual bool stop() = 0;
    virtual bool quit() = 0;
    virtual bool volu() = 0;
    virtual bool vold() = 0;
    virtual bool rewd() = 0;
    virtual bool frwd() = 0;
    virtual bool jump(int) = 0;
    virtual bool volume(int) = 0;
    virtual bool showPlayer() = 0;
    virtual bool openUri(QStringList) = 0;
    virtual bool appendFile(QStringList) = 0;

protected slots:
    virtual void update() = 0;
    virtual void getInfo() = 0;
};

#endif // PLAYERINTERFACE_H
