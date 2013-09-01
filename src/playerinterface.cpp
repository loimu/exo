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

#include <QProcess>
#include <QTimer>

#include "playerinterface.h"

PlayerInterface *PlayerInterface::m_instance=0;

PlayerInterface::PlayerInterface(QObject *parent) :
    QObject(parent) {
    if(m_instance)
        qFatal("PlayerInterface: only one instance is allowed");
    m_instance = this;

    m_listened = true;
    m_nowPlaying = "";
    artist = "";
    title = "";

    if(!isServerRunning())
        runServer();

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(1000);
}

PlayerInterface::~PlayerInterface() {
    m_instance = 0;
}

bool PlayerInterface::isServerRunning() {
    QProcess proc;
    proc.start("pidof", QStringList() << "mocp");
    proc.waitForFinished(-1);
    QString output = QString::fromUtf8(proc.readAllStandardOutput());
    if(output.length() > 1)
        return true;
    else
        return false;
}

void PlayerInterface::runServer() {
    QProcess proc;
    proc.startDetached("mocp", QStringList() << "-S");
}

void PlayerInterface::play() {
    QProcess proc;
    proc.startDetached("mocp", QStringList() << "-p");
}

void PlayerInterface::pause() {
    QProcess proc;
    proc.startDetached("mocp", QStringList() << "-G");
}

void PlayerInterface::prev() {
    QProcess proc;
    proc.startDetached("mocp", QStringList() << "-r");
}

void PlayerInterface::next() {
    QProcess proc;
    proc.startDetached("mocp", QStringList() << "-f");
}

void PlayerInterface::stop() {
    QProcess proc;
    proc.startDetached("mocp", QStringList() << "-s");
}

void PlayerInterface::quit() {
    QProcess proc;
    proc.startDetached("mocp", QStringList() << "-x");
}

void PlayerInterface::update() {
    QProcess proc;
    proc.start("mocp", QStringList() << "-i");
    proc.waitForFinished(-1);
    QString output = QString::fromUtf8(proc.readAllStandardOutput());
    m_list = output.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
    m_list.replaceInStrings(QRegExp("(\\w+:\\s)+(.*)"), "\\2");
//    for (int i = 0; i < list.size(); ++i) {
//        qDebug("debug: %s", qPrintable(list.at(i)));
//    }
    if(m_list.size() >= 11) {
        int currentTime = m_list.at(10).toInt();
        int totalTime = m_list.at(8).toInt();

        title = m_list.at(4);
        if(artist.isEmpty() && !title.isEmpty()) {
            artist = title;
            artist.replace(QRegExp("^(.+)\\s-\\s.*"), "\\1");
            title.replace(QRegExp("^.+\\s-\\s(.*)"), "\\1");
            totalSec = 8*60;
        }
        else {
            artist = m_list.at(3);
            totalSec = m_list.at(8).toInt();
        }

        if(m_nowPlaying != m_list.at(2)) {
            m_nowPlaying = m_list.at(2);
            emit trackChanged();
        }
        else if(m_listened && ((currentTime < totalTime/2 && totalTime < 8*60)||
                             (currentTime < 4*60 && totalTime > 8*60))) {
            m_listened = false;
        }
        else if(!m_listened && (currentTime > totalTime/2 ||
                            (currentTime > 4*60 && totalTime > 8*60))) {
            m_listened = true;
            emit trackListened();
        }
    }
    updateStatus();
}

void PlayerInterface::openWindow() {
    QProcess proc;
    proc.startDetached("x-terminal-emulator", QStringList() << "-e" << "mocp");
}
