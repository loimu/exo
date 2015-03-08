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

#include <QTimer>
#include <QProcess>
#include <QDir>

#include "playerinterface.h"

PlayerInterface* PlayerInterface::object = 0;

PlayerInterface::PlayerInterface(QObject* parent) : QObject(parent), track()
{
    if(object)
        qFatal("only one instance is allowed");
    object = this;
}

void PlayerInterface::startTimer(int period) {
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(period);
}

QString PlayerInterface::execute(QString program, QStringList options) {
    QProcess proc;
    proc.start(program, options);
    proc.waitForFinished(-1);
    return QString::fromUtf8(proc.readAllStandardOutput());
}

void PlayerInterface::scrobbler() {
    static QString nowPlaying = QString();
    static bool listened = true;
    if(nowPlaying != track.title && !track.artist.isEmpty()) {
        nowPlaying = track.title;
        emit trackChanged(track.artist, track.song, track.totalSec);
        return;
    }
    if(track.file.startsWith("http"))
        return;
    if(listened && ((track.currSec < track.totalSec/2 && track.totalSec < 8*60)
                    || (track.currSec < 4*60 && track.totalSec > 8*60))) {
        listened = false;
    }
    else if(!listened && (track.currSec > track.totalSec/2 ||
                          (track.currSec > 4*60 && track.totalSec > 8*60))) {
        listened = true;
        emit trackListened(track.artist,track.song,track.album,track.totalSec);
    }
}

QString PlayerInterface::cover() {
    if(track.file.startsWith("http"))
        return "";
    QString path = track.file;
    path.replace(QRegExp("(.*)/(.*)"), "\\1");
    QDir dir(path);
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg";
    dir.setNameFilters(filters);
    if(dir.entryList().size() > 0)
        return path + "/" + dir.entryList().at(0);
    else
        return ":/images/nocover.png";
}

QString PlayerInterface::artist() {
    return track.artist;
}

QString PlayerInterface::title() {
    return track.song;
}

PlayerInterface* PlayerInterface::instance() {
    return object;
}
