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

#include <QStringList>

#include "playerinterface.h"
#include "mocplayerinterface.h"

#define OSD_OPT "OnSongChange=\"/usr/bin/moc-osd.py\""

MOCPlayerInterface::MOCPlayerInterface(QObject *parent) :PlayerInterface(parent)
{
    if(!isServerRunning())
        runServer();
    startTimer(1000);
}

bool MOCPlayerInterface::isServerRunning() {
    return execute("pidof", QStringList() << "mocp").length() > 1;
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

void MOCPlayerInterface::getInfo() {
    QString info = execute("mocp", QStringList() << "-Q" << "\"<st>%state</st>"
                           "<ar>%artist</ar><sn>%song</sn><al>%album</al>"
                           "<fi>%file</fi><tt>%tt</tt><ct>%ct</ct><ts>%ts</ts>"
                           "<cs>%cs</cs><np>%title</np>\" 2> /dev/null");
    if(info.size() < 1) {
        track.state = "Offline";
        return;
    }
    QRegExp infoRgx("<st>(.*)</st><ar>(.*)</ar><sn>(.*)</sn><al>(.*)</al>"
                    "<fi>(.*)</fi><tt>(.*)</tt><ct>(.*)</ct><ts>(.*)</ts>"
                    "<cs>(.*)</cs><np>(.*)</np>");
    infoRgx.setMinimal(true);
    infoRgx.indexIn(info);
    track.state = infoRgx.cap(1);
    if(track.state == "STOP")
        return;
    track.artist = infoRgx.cap(2);
    track.song = infoRgx.cap(3);
    track.album = infoRgx.cap(4);
    track.file = infoRgx.cap(5);
    track.totalTime = infoRgx.cap(6);
    track.currTime = infoRgx.cap(7);
    track.totalSec = infoRgx.cap(8).toInt();
    track.currSec = infoRgx.cap(9).toInt();
    track.title = infoRgx.cap(10);
    if(!track.file.startsWith("http"))
        return;
    track.totalSec = 8*60;
    if(!track.title.isEmpty()) {
        QRegExp artistRgx("^(.*)\\s-\\s");
        artistRgx.setMinimal(true);
        artistRgx.indexIn(track.title);
        track.artist = artistRgx.cap(1);
        QRegExp titleRgx("\\s-\\s(.*)$");
        titleRgx.indexIn(track.title);
        track.song = titleRgx.cap(1);
    }
    else {
        track.title = track.file;
    }
}

void MOCPlayerInterface::update() {
    getInfo();
    if(track.state == "Offline") {
        emit updateStatus(tr("Player isn't running. "
                             "Double-click to run it."), "", "", "");
        return;
    }
    if(track.state == "STOP") {
        emit updateStatus(tr("Stopped"), "", "", "");
        return;
    }
    emit updateStatus(track.title, track.currTime, track.totalTime, cover());
    if(track.state != "PAUSE")
        scrobbler();
}

void MOCPlayerInterface::showPlayer() {
    QString term = "x-terminal-emulator";
    // falling back to xterm if there's no "alternatives"
    if(!(execute("which", QStringList() << term).length() > 1))
        term = "xterm";
    execute(term, QStringList() << "-e" << "mocp" << "-O" << OSD_OPT);
}
