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

#include "core/process.h"
#include "mocinterface.h"

#define OSD_OPT "OnSongChange=\"/usr/share/exo/moc-osd.py\""

MocInterface::MocInterface(QObject *parent) :PlayerInterface(parent)
{
    if(!isServerRunning())
        runServer();
    startTimer(1000);
}

QString MocInterface::id() {
    return QLatin1String("music on console");
}

bool MocInterface::isServerRunning() {
    return !Process::getOutput(
                QLatin1String("pidof"),
                QStringList{QLatin1String("mocp")}).isEmpty();
}

bool MocInterface::runServer() {
#ifdef OSD_OPT
    return Process::execute(
                QLatin1String("mocp"),
                QStringList{QLatin1String("-SO"), QLatin1String(OSD_OPT)});
#else //OSD_OPT
    return Process::execute(
                QLatin1String("mocp"), QStringList{QLatin1String("-S")});
#endif // OSD_OPT
}

#define SEND_COMMAND(__method, __option)\
    bool MocInterface::__method() {\
    return Process::execute(QLatin1String("mocp"), QStringList{__option});\
    }

SEND_COMMAND(play, QLatin1String("-p"))
SEND_COMMAND(pause,QLatin1String("-P"))
SEND_COMMAND(playPause, QLatin1String("-G"))
SEND_COMMAND(prev, QLatin1String("-r"))
SEND_COMMAND(next, QLatin1String("-f"))
SEND_COMMAND(stop, QLatin1String("-s"))
SEND_COMMAND(quit, QLatin1String("-x"))

#define SEND_COMMAND_PARAM(__method, __option)\
    bool MocInterface::__method(int param) {\
    return Process::execute(QLatin1String("mocp"),\
    QStringList() << QString(__option).arg(param));\
    }

SEND_COMMAND_PARAM(jump, QLatin1String("-j%1s"))
SEND_COMMAND_PARAM(seek, QLatin1String("-k%1"))
SEND_COMMAND_PARAM(volume, QLatin1String("-v%1"))
SEND_COMMAND_PARAM(changeVolume, QLatin1String("-v+%1"))

bool MocInterface::showPlayer() {
    QString term = QLatin1String("xterm"); // xterm is a fallback app
    QStringList apps = Process::detect(
                QStringList{
                    QLatin1String("x-terminal-emulator"),
                    QLatin1String("gnome-terminal"),
                    QLatin1String("konsole"),
                    QLatin1String("xfce4-terminal"),
                    QLatin1String("lxterminal")});
    if(!apps.isEmpty())
        term = apps.at(0);
#ifdef OSD_OPT
    return Process::execute(
                term, QStringList{
                    QLatin1String("-e"),
                    QLatin1String("mocp"),
                    QLatin1String("-O"),
                    QLatin1String(OSD_OPT)});
#else // OSD_OPT
    return Process::execute(
                term, QStringList{QLatin1String("-e"), QLatin1String("mocp")});
#endif // OSD_OPT
}

bool MocInterface::openUri(const QString file) {
    return Process::execute(QLatin1String("mocp"),
                            QStringList() << QLatin1String("-l") << file);
}

bool MocInterface::appendFile(QStringList files) {
    return Process::execute(QLatin1String("mocp"),
                            QStringList() << QLatin1String("-a") << files);
}

State MocInterface::getInfo() {
    QString info = Process::getOutput(
                QLatin1String("mocp"),
                QStringList{
                    QLatin1String("-Q"),
                    QLatin1String("\"{s}%state{a}%a{t}%t{A}%A{f}%file"
                    "{n}%n{tt}%tt{ct}%ct{ts}%ts{cs}%cs{T}%title{end}\"")});
    if(info.isEmpty())
        return Offline;
    QRegExp infoRgx(
                QLatin1String("\\{s\\}(.*)\\{a\\}(.*)\\{t\\}(.*)\\{A\\}(.*)"
                              "\\{f\\}(.*)\\{n\\}(.*)\\{tt\\}(.*)\\{ct\\}(.*)"
                              "\\{ts\\}(.*)\\{cs\\}(.*)\\{T\\}(.*)\\{end\\}"));
    infoRgx.setMinimal(true);
    infoRgx.indexIn(info);
    if(infoRgx.cap(1) == QLatin1String("STOP"))
        return Stop;
    State state = Offline;
    if(infoRgx.cap(1) == QLatin1String("PLAY"))
        state = Play;
    else if(infoRgx.cap(1) == QLatin1String("PAUSE"))
        state = Pause;
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
    if(!track.file.startsWith(QLatin1String("http")))
        return state;
    track.totalSec = 8*60;
    if(!track.title.isEmpty()) {
        QRegExp artistRgx(QLatin1String("^(.*)\\s-\\s"));
        artistRgx.setMinimal(true);
        artistRgx.indexIn(track.title);
        track.artist = artistRgx.cap(1);
        QRegExp titleRgx(QLatin1String("\\s-\\s(.*)$"));
        titleRgx.indexIn(track.title);
        track.song = titleRgx.cap(1);
    }
    return state;
}
