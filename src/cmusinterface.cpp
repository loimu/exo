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

#include "cmusinterface.h"

CmusInterface::CmusInterface(QObject *parent) : PlayerInterface(parent)
{
    if(!isPlayerRunning())
        runPlayer();
    startTimer(1000);
}

CmusInterface::~CmusInterface()
{
}

QString CmusInterface::id() {
    return "Cmus";
}

bool CmusInterface::isPlayerRunning() {
    return getOutput("pidof", QStringList() << "cmus").length() > 1;
}

bool CmusInterface::runPlayer() {
    QString term = "x-terminal-emulator";
    // falling back to xterm if there's no "alternatives"
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
    QString o = (offset > 0) ? "+" + offset : QString::number(offset);
    return execute("cmus-remote", QStringList() << "-k" << o);
}

bool CmusInterface::volume(int lev) {
    return execute("cmus-remote", QStringList() << "-k" <<QString::number(lev));
}

bool CmusInterface::changeVolume(int delta) {
    QString d = (delta > 0) ? "+" + delta : QString::number(delta);
    return execute("cmus-remote", QStringList() << "-k" << d);
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

void CmusInterface::getInfo() {
    QString info = getOutput("cmus-remote", QStringList() << "-Q");
    if(info.size() < 1) {
        track.state = "Offline";
        return;
    }
    track.state = find(info, "status\\s(.*)\\n");
    if(track.state == "stopped")
        return;
    track.artist = find(info, "tag\\sartist\\s(.*)\\n");
    track.song = find(info, "tag\\stitle\\s(.*)\\n");
    track.album = find(info, "tag\\salbum\\s(.*)\\n");
    track.file = find(info, "file\\s(.*)\\n");
    track.totalTime = find(info, "duration\\s(.*)\\n");
    track.currTime = find(info, "position\\s(.*)\\n");
    track.totalSec = track.totalTime.toInt();
    track.currSec = track.currTime.toInt();
    track.title = track.song;
    track.number = find(info, "tag\\stracknumber\\s(.*)\\n").toInt();
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
