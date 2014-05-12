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

#include <QTimer>
#include <QProcess>

#include "playerinterface.h"

PlayerInterface* PlayerInterface::object = 0;

PlayerInterface::PlayerInterface(QObject* parent) : QObject(parent),
    artistString(QString()), titleString(QString()) {
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

QString PlayerInterface::artist() {
    return artistString;
}

QString PlayerInterface::title() {
    return titleString;
}

PlayerInterface* PlayerInterface::instance() {
    return object;
}
