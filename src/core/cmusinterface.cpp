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
#include <QTime>

#include "cmusinterface.h"

CmusInterface::CmusInterface(QObject *parent) : PlayerInterface(parent)
{
    if(!isPlayerRunning())
        runPlayer();
    startTimer(1000);
}

QString CmusInterface::id() {
    return "Cmus";
}

bool CmusInterface::isPlayerRunning() {
    return getOutput("pidof", QStringList() << "cmus").length() > 1;
}

bool CmusInterface::runPlayer() {
    QString term = "x-terminal-emulator";
    // falling back to xterm if there're no alternatives
    if(getOutput("which", QStringList() << term).length() < 1)
        term = "xterm";
    return execute(term, QStringList() << "-e" << "cmus");
}

#define SEND_COMMAND(__method, __option)\
    bool CmusInterface::__method() {\
        return execute("cmus-remote", QStringList() << __option);\
    }

SEND_COMMAND(play, "-p")
SEND_COMMAND(pause,"-u")
SEND_COMMAND(playPause,"-u")
SEND_COMMAND(prev, "-r")
SEND_COMMAND(next, "-n")
SEND_COMMAND(stop, "-s")

bool CmusInterface::quit() {
    return execute("cmus-remote", QStringList() << "-C" << "quit");
}

bool CmusInterface::jump(int pos) {
    return execute("cmus-remote", QStringList() << "-k" <<QString::number(pos));
}

bool CmusInterface::seek(int offset) {
    QString o = (offset > 0) ? QLatin1String("+") + QString::number(offset) : QString::number(offset);
    return execute("cmus-remote", QStringList() << "-k" << o);
}

bool CmusInterface::volume(int lev) {
    return execute("cmus-remote", QStringList() << "-v" << QString::number(lev)
                   + QLatin1String("%"));
}

bool CmusInterface::changeVolume(int delta) {
    QString d = ((delta > 0) ? QLatin1String("+") + QString::number(delta) : QString::number(delta))
            + QLatin1String("%");
    return execute("cmus-remote", QStringList() << "-v" << d);
}

bool CmusInterface::showPlayer() {
    return runPlayer();
}

bool CmusInterface::openUri(const QString file) {
    return execute("cmus-remote", QStringList() << "-q" << file);
}

bool CmusInterface::appendFile(QStringList files) {
    return execute("cmus-remote", QStringList() << "-q" << files);
}

QString CmusInterface::find(QString string, const QString regEx) {
    QRegExp findRgx(regEx);
    findRgx.setMinimal(true);
    findRgx.indexIn(string);
    return findRgx.cap(1);
}

State CmusInterface::getInfo() {
    QString info = getOutput("cmus-remote", QStringList() << "-Q");
    if(info.size() < 1)
        return Offline;
    QString string = find(info, "status\\s(.*)\\n");
    if(string == "stopped")
        return Stop;
    State state = Offline;
    if(string == "playing")
        state = Play;
    if(string == "paused")
        state = Pause;
    track.artist = find(info, "tag\\sartist\\s(.*)\\n");
    track.song = find(info, "tag\\stitle\\s(.*)\\n");
    track.album = find(info, "tag\\salbum\\s(.*)\\n");
    track.file = find(info, "file\\s(.*)\\n");
    track.totalSec = find(info, "duration\\s(.*)\\n").toInt();
    track.currSec = find(info, "position\\s(.*)\\n").toInt();
    track.totalTime = QTime().addSecs(track.totalSec).toString("mm:ss");
    track.currTime = QTime().addSecs(track.currSec).toString("mm:ss");
    track.number = find(info, "tag\\stracknumber\\s(.*)\\n").toInt();
    track.title = track.artist.isEmpty() ? track.song : track.artist + " - " + track.song;
    if(!track.file.startsWith("http"))
        return state;
    QString song = find(info, "stream\\s(.*)\\n");
    track.title += QLatin1String("<br />") + song;
    track.totalSec = 8*60;
    if(!song.isEmpty()) {
        QRegExp artistRgx("^(.*)\\s-\\s");
        artistRgx.setMinimal(true);
        artistRgx.indexIn(song);
        track.artist = artistRgx.cap(1);
        QRegExp titleRgx("\\s-\\s(.*)$");
        titleRgx.indexIn(song);
        track.song = titleRgx.cap(1);
    }
    return state;
}
