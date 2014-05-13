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

#include <QStringList>

#include "playerinterface.h"
#include "mocplayerinterface.h"

#define OSD_OPT "OnSongChange=\"/usr/bin/moc-osd.py\""

MOCPlayerInterface::MOCPlayerInterface(QObject *parent) :
    PlayerInterface(parent) {
    if(!isServerRunning())
        runServer();
    startTimer(1000);
}

bool MOCPlayerInterface::isServerRunning() {
    if(execute("pidof", QStringList() << "mocp").length() > 1)
        return true;
    else
        return false;
}

void MOCPlayerInterface::sendOption(QString option) {
    execute("mocp", QStringList() << option);
}

void MOCPlayerInterface::runServer() {
    execute("mocp", QStringList() << "-SO" << OSD_OPT);
}

void MOCPlayerInterface::play() {
    sendOption("-p");
}

void MOCPlayerInterface::pause() {
    sendOption("-G");
}

void MOCPlayerInterface::prev() {
    sendOption("-r");
}

void MOCPlayerInterface::next() {
    sendOption("-f");
}

void MOCPlayerInterface::stop() {
    sendOption("-s");
}

void MOCPlayerInterface::quit() {
    sendOption("-x");
}

void MOCPlayerInterface::volu() {
    sendOption("-v+2");
}

void MOCPlayerInterface::vold() {
    sendOption("-v-2");
}

void MOCPlayerInterface::rewd() {
    sendOption("-k-10");
}

void MOCPlayerInterface::frwd() {
    sendOption("-k10");
}

void MOCPlayerInterface::appendFile(QString file) {
    execute("mocp", QStringList() << "-a" << file);
}

void MOCPlayerInterface::update() {
    QStringList list = execute("mocp", QStringList() << "-i")
            .split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
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
            titleString = list.at(4);
            if(message.isEmpty())
                message = path;
            // condition is true for radio streams
            if(listSize == streamListSize) {
                totalSec = 8*60;
                if(!titleString.isEmpty()) {
                    QRegExp artistRgx("^(.*)\\s-\\s");
                    artistRgx.setMinimal(true);
                    QRegExp titleRgx("\\s-\\s(.*)$");
                    artistRgx.indexIn(titleString);
                    titleRgx.indexIn(titleString);
                    artistString = artistRgx.cap(1);
                    titleString = titleRgx.cap(1);
                }
            }
            else {
                artistString = list.at(3);
                totalSec = list.at(8).toInt();
                totalTime = list.at(6);
            }
            // signal for scrobbler
            if(!titleString.isEmpty())
                emit trackChanged(artistString, titleString, totalSec);
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
                emit trackListened(artistString, titleString, album, totalSec);
            }
        }
    }
    else {
        artistString = QString();
        titleString = QString();
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

void MOCPlayerInterface::openWindow() {
    execute("x-terminal-emulator", QStringList() << "-e" << "mocp" << "-O"
            << OSD_OPT);
}
