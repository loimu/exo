/* ========================================================================
*    Copyright (C) 2013-2016 Blaze <blaze@vivaldi.net>
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

#include "mocplayerinterface.h"

#define OSD_OPT "OnSongChange=\"/usr/share/exo/moc-osd.py\""

MOCPlayerInterface::MOCPlayerInterface(QObject *parent) :PlayerInterface(parent)
{
    if(!isServerRunning())
        runServer();
    startTimer(1000);
}

MOCPlayerInterface::~MOCPlayerInterface()
{
}

QString MOCPlayerInterface::id() {
    return "music on console";
}

bool MOCPlayerInterface::isServerRunning() {
    return getOutput("pidof", QStringList() << "mocp").length() > 1;
}

bool MOCPlayerInterface::runServer() {
#ifdef OSD_OPT
    return execute("mocp", QStringList() << "-SO" << OSD_OPT);
#else //OSD_OPT
    return execute("mocp", QStringList() << "-S");
#endif // OSD_OPT
}

#define SEND_COMMAND(__method, __option)\
    bool MOCPlayerInterface::__method() {\
        return execute("mocp", QStringList() << __option);\
    }

SEND_COMMAND(play, "-p")
SEND_COMMAND(pause,"-P")
SEND_COMMAND(playPause,"-G")
SEND_COMMAND(prev, "-r")
SEND_COMMAND(next, "-f")
SEND_COMMAND(stop, "-s")
SEND_COMMAND(quit, "-x")

#define SEND_COMMAND_PARAM(__method, __option)\
    bool MOCPlayerInterface::__method(int param) {\
        return execute("mocp", QStringList() << QString(__option).arg(param));\
    }

SEND_COMMAND_PARAM(jump, "-j%1s")
SEND_COMMAND_PARAM(seek, "-k%1")
SEND_COMMAND_PARAM(volume, "-v%1")
SEND_COMMAND_PARAM(changeVolume, "-v+%1")

bool MOCPlayerInterface::showPlayer() {
    QString term = "x-terminal-emulator";
    // falling back to xterm if there's no "alternatives"
    if(getOutput("which", QStringList() << term).length() < 1)
        term = "xterm";
#ifdef OSD_OPT
    return execute(term, QStringList() << "-e" << "mocp" << "-O" << OSD_OPT);
#else // OSD_OPT
    return execute(term, QStringList() << "-e" << "mocp");
#endif // OSD_OPT
}

bool MOCPlayerInterface::openUri(const QString file) {
    return execute("mocp", QStringList() << "-l" << file);
}

bool MOCPlayerInterface::appendFile(QStringList files) {
    return execute("mocp", QStringList() << "-a" << files);
}

void MOCPlayerInterface::getInfo() {
    QString info = getOutput("mocp", QStringList() << "-Q" << "\"{s}%state{a}%a"
        "{t}%t{A}%A{f}%file{n}%n{tt}%tt{ct}%ct{ts}%ts{cs}%cs{T}%title{end}\"");
    if(info.size() < 1) {
        track.state = "Offline";
        return;
    }
    QRegExp infoRgx("\\{s\\}(.*)\\{a\\}(.*)\\{t\\}(.*)\\{A\\}(.*)\\{f\\}(.*)"
                    "\\{n\\}(.*)\\{tt\\}(.*)\\{ct\\}(.*)\\{ts\\}(.*)"
                    "\\{cs\\}(.*)\\{T\\}(.*)\\{end\\}");
    infoRgx.setMinimal(true);
    infoRgx.indexIn(info);
    track.state = infoRgx.cap(1);
    if(track.state == "STOP")
        return;
    track.artist = infoRgx.cap(2);
    track.song = infoRgx.cap(3);
    track.album = infoRgx.cap(4);
    track.file = infoRgx.cap(5);
    track.number = infoRgx.cap(6).toInt();
    track.totalTime = infoRgx.cap(7);
    track.currTime = infoRgx.cap(8);
    track.totalSec = infoRgx.cap(9).toInt();
    track.currSec = infoRgx.cap(10).toInt();
    track.title = infoRgx.cap(11);
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
}
