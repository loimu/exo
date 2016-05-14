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

#include <QTimer>
#include <QProcess>
#include <QDir>

#include "playerinterface.h"

PlayerInterface* PlayerInterface::object = nullptr;

PlayerInterface::PlayerInterface(QObject* parent) : QObject(parent), track()
{
    if(object)
        qFatal("PlayerInterface: only one instance is allowed");
    object = this;
}

void PlayerInterface::startTimer(int period) {
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(period);
}

QString PlayerInterface::getOutput(QString program, QStringList options) {
    QProcess proc;
    proc.start(program, options);
    proc.waitForFinished(-1);
    return QString::fromUtf8(proc.readAllStandardOutput());
}

bool PlayerInterface::execute(QString program, QStringList options) {
    QProcess proc;
    proc.start(program, options);
    proc.waitForFinished(-1);
    // returns true (success) if no output in standard error
    return proc.readAllStandardError().length() < 1;
}

void PlayerInterface::scrobble() {
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

void PlayerInterface::update() {
    getInfo();
#ifdef BUILD_DBUS
    static QString title = QString();
    if(title != track.title) {
        title = track.title;
        emit newTrack();
    }
#endif // BUILD_DBUS
    static QString status = QString();
    if(status != track.state) {
        status = track.state;
        emit newStatus(status);
        if(status == "Offline")
            emit updateStatus(tr("Player isn't running."), "", "", "");
        if(status.startsWith("stop", Qt::CaseInsensitive))
            emit updateStatus(tr("Stopped"), "", "", "");
    }
    if(track.state.startsWith("play", Qt::CaseInsensitive)) {
        emit updateStatus(track.title, track.currTime, track.totalTime,cover());
        scrobble();
    }
}

#ifdef BUILD_DBUS
QString PlayerInterface::artwork() {
    // compliance method for MPRIS
    QString art = cover();
    return (art == ":/images/nocover.png" || art == "") ? "" : "file://" + art;
}
#endif // BUILD_DBUS

PlayerInterface* PlayerInterface::instance() {
    return object;
}

const Track* PlayerInterface::trackObject() const {
    return &track;
}
