/* ========================================================================
*    Copyright (C) 2013-2017 Blaze <blaze@vivaldi.net>
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

#include "config.h"

#include <QStringList>
#include <QSettings>

#include "core/process.h"
#include "mocinterface.h"

#define OSD_OPT "OnSongChange=" CMAKE_INSTALL_PREFIX "/share/exo/moc-osd.py"

MocInterface::MocInterface(QObject *parent) : PlayerInterface(parent)
{
    if(!isServerRunning())
        runServer();
    startTimer(1000);
}

MocInterface::~MocInterface()
{
    QSettings settings;
    if(settings.value(QLatin1String("player/quit")).toBool())
        quit();
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
    return Process::execute(
                QLatin1String("mocp"),
                QStringList{QLatin1String("-SO"), QLatin1String(OSD_OPT)});
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

void MocInterface::showPlayer() {
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
    Process::execute(term, QStringList{
                         QLatin1String("-e"),
                         QLatin1String("mocp -O " OSD_OPT)});
}

bool MocInterface::openUri(const QString& file) {
    return Process::execute(QLatin1String("mocp"),
                            QStringList() << QLatin1String("-l") << file);
}

bool MocInterface::appendFile(const QStringList& files) {
    return Process::execute(QLatin1String("mocp"),
                            QStringList() << QLatin1String("-a") << files);
}

State MocInterface::getInfo() {
    QString info = Process::getOutput(
                QLatin1String("mocp"),
                QStringList{
                    QLatin1String("-Q"),
                    QLatin1String("%state{a}%a{t}%t{A}%A{f}%file{tt}%tt{ts}%ts"
                    "{cs}%cs{T}%title")});
    if(info.isEmpty())
        return Offline;
    if(info.startsWith(QLatin1String("STOP")))
        return Stop;
    QRegExp infoRgx(
                QLatin1String("^(.*)\\{a\\}(.*)\\{t\\}(.*)\\{A\\}(.*)"
                              "\\{f\\}(.*)\\{tt\\}(.*)\\{ts\\}(.*)"
                              "\\{cs\\}(.*)\\{T\\}(.*)\n"));
    infoRgx.setMinimal(true);
    infoRgx.indexIn(info);
    State state = Offline;
    if(infoRgx.cap(1) == QLatin1String("PLAY"))
        state = Play;
    else if(infoRgx.cap(1) == QLatin1String("PAUSE"))
        state = Pause;
    track.artist = infoRgx.cap(2);
    track.title = infoRgx.cap(3);
    track.album = infoRgx.cap(4);
    track.file = infoRgx.cap(5);
    track.totalTime = infoRgx.cap(6);
    track.totalSec = infoRgx.cap(7).toInt();
    track.currSec = infoRgx.cap(8).toInt();
    track.caption = infoRgx.cap(9);
    track.isStream = !track.file.startsWith(QLatin1Char('/'));
    if(!track.isStream) {
        track.caption.append(QString(" (%1)").arg(track.totalTime));
        return state;
    }
    track.totalSec = 8*60;
    if(!track.caption.isEmpty()) {
        QRegExp artistRgx(QLatin1String("^(.*)\\s-\\s"));
        artistRgx.setMinimal(true);
        artistRgx.indexIn(track.caption);
        track.artist = artistRgx.cap(1);
        QRegExp titleRgx(QLatin1String("\\s-\\s(.*)$"));
        titleRgx.indexIn(track.caption);
        track.title = titleRgx.cap(1);
    } else
        track.caption = track.file;
    return state;
}
