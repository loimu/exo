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

#include <QProcess>

#include "process.h"

QString Process::getOutput(const QString& program, const QStringList& options) {
    QProcess proc;
    proc.start(program, options);
    proc.waitForFinished(-1);
    return QString::fromUtf8(proc.readAllStandardOutput());
}

bool Process::execute(const QString& program, const QStringList& options) {
    QProcess proc;
    proc.start(program, options);
    proc.waitForFinished(-1);
    /* returns true (success) if no output at stderr */
    return proc.readAllStandardError().isEmpty();
}

QStringList Process::detect(const QStringList& apps) {
    QProcess proc;
    proc.start(QLatin1String("which"), apps);
    proc.waitForFinished(-1);
    return QString::fromUtf8(proc.readAllStandardOutput())
            .split(QLatin1String("\n"), QString::SkipEmptyParts);
}
