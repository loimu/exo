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

protected:
    void startTimer(int);
    QString execute(QString, QStringList);
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
    virtual void play() {}
    virtual void pause() {}
    virtual void prev() {}
    virtual void next() {}
    virtual void stop() {}
    virtual void quit() {}
    virtual void volu() {}
    virtual void vold() {}
    virtual void rewd() {}
    virtual void frwd() {}
    virtual void openWindow() {}
    virtual void appendFile(QString) {}

protected slots:
    virtual void update() {}
};

#endif // PLAYERINTERFACE_H
