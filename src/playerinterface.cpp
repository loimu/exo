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

#include <QProcess>
#include <QTimer>
#include <QSettings>

#include "exo.h"
#include "playerinterface.h"

PlayerInterface* PlayerInterface::m_instance = 0;

PlayerInterface::PlayerInterface(QObject* parent) : QObject(parent),
    m_artist(QString()), m_title(QString()) {
    if(m_instance)
        qFatal("only one instance is allowed");
    m_instance = this;
    if(!isServerRunning())
        runServer();
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(1000);
}

bool PlayerInterface::isServerRunning() {
    if(execute("pidof", "mocp").length() > 1)
        return true;
    else
        return false;
}

QString PlayerInterface::execute(QString program, QString option,
                                 QString secondOption) {
    QProcess proc;
    proc.start(program, QStringList() << option << secondOption);
    proc.waitForFinished(-1);
    return QString::fromUtf8(proc.readAllStandardOutput());
}

void PlayerInterface::sendOption(QString option) {
    execute("mocp", option);
}

void PlayerInterface::runServer() {
    sendOption("-S");
}

void PlayerInterface::play() {
    sendOption("-p");
}

void PlayerInterface::pause() {
    sendOption("-G");
}

void PlayerInterface::prev() {
    sendOption("-r");
}

void PlayerInterface::next() {
    sendOption("-f");
}

void PlayerInterface::stop() {
    sendOption("-s");
}

void PlayerInterface::quit() {
    QSettings* settings = Exo::app()->settings();
    if(settings->value("player/quit").toBool())
        sendOption("-x");
}

void PlayerInterface::volu() {
    sendOption("-v+2");
}

void PlayerInterface::vold() {
    sendOption("-v-2");
}

void PlayerInterface::rewd() {
    sendOption("-k-10");
}

void PlayerInterface::frwd() {
    sendOption("-k10");
}

void PlayerInterface::appendFile(QString file) {
    execute("mocp", "-a", file);
}

void PlayerInterface::update() {
    QStringList list = execute("mocp", "-i").split(QRegExp("[\r\n]"),
                                                   QString::SkipEmptyParts);
    list.replaceInStrings(QRegExp("(\\w+:\\s)+(.*)"), "\\2");
    int listSize = list.size();
    static bool listened = true;
    static QString message = QString();
    static QString totalTime = QString();
    static QString path = QString();
    static QString nowPlaying = QString();
    static int totalSec = 0;
    static const int streamListSize = 11;
    QString currentTime = QString();
    // the following condition is true if file or stream is playing
    if(listSize >= streamListSize) {
        int currentSec = list.at(10).toInt();
        currentTime = list.at(9);
        // condition is true if track have changed
        if(path != list.at(1) || nowPlaying != list.at(2)) {
            path = list.at(1);
            nowPlaying = list.at(2);
            message = list.at(2);
            m_title = list.at(4);
            if(message.isEmpty())
                message = path;
            // condition is true for radio streams
            if(listSize == streamListSize) {
                totalSec = 8*60;
                if(!m_title.isEmpty()) {
                    QRegExp artistRgx("^(.*)\\s-\\s");
                    artistRgx.setMinimal(true);
                    QRegExp titleRgx("\\s-\\s(.*)$");
                    artistRgx.indexIn(m_title);
                    titleRgx.indexIn(m_title);
                    m_artist = artistRgx.cap(1);
                    m_title = titleRgx.cap(1);
                }
            }
            else {
                m_artist = list.at(3);
                totalSec = list.at(8).toInt();
                totalTime = list.at(6);
            }
            // signal for scrobbler
            if(!m_title.isEmpty())
                emit trackChanged(m_artist, m_title, totalSec);
        }
        else if(listSize > streamListSize) {
            if(listened && ((currentSec < totalSec/2 && totalSec < 8*60)||
                                   (currentSec < 4*60 && totalSec > 8*60))) {
                listened = false;
            }
            else if(!listened && (currentSec > totalSec/2 ||
                                    (currentSec > 4*60 && totalSec > 8*60))) {
                listened = true;
                QString album = list.at(5);
                // signal for scrobbler
                emit trackListened(m_artist, m_title, album, totalSec);
            }
        }
    }
    else {
        m_artist = QString();
        m_title = QString();
        path = QString();
        if(listSize == 0)
            message = tr("Player is not running, make a doubleclick.");
        else if (listSize == 1)
            message = tr("Stopped");
    }
    QString mediaPath;
    if(path.startsWith("/"))
        mediaPath = path;
    // signal for trayicon
    emit updateStatus(message, currentTime, totalTime, mediaPath);
}

void PlayerInterface::openWindow() {
    execute("x-terminal-emulator", "-e", "mocp");
}

QString PlayerInterface::artist() {
    return QString(m_artist);
}

QString PlayerInterface::title() {
    return QString(m_title);
}

PlayerInterface* PlayerInterface::instance() {
    return m_instance;
}
