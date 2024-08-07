/* ========================================================================
*    Copyright (C) 2013-2024 Blaze <blaze@vivaldi.net>
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

#include <QString>
#include <QFile>
#include <QDir>

#include <QDebug>

#include "sysutils.h"


const int SysUtils::findProcessId(const QString& name) {
    const QDir procDir(QStringLiteral("/proc"));
    const QStringList pids = procDir.entryList(
                QStringList { QStringLiteral("[0-9]?*") },
                QDir::Dirs|QDir::NoSymLinks|QDir::NoDotAndDotDot);
    for(const QString& pid : pids) {
        QFile file(QString(QStringLiteral("/proc/%1/comm")).arg(pid));
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            const QString ll = file.readLine().trimmed();
            if(ll == name) {
                return pid.toInt();
            }
        }
    }
    return -1;
}

const QVector<QString> SysUtils::findFullPaths(
        const QVector<QString>& executables) {
    const QStringList paths = QString(QLatin1String(qgetenv("PATH")))
                                  .split(QChar::fromLatin1(':'), Qt::SkipEmptyParts);
    QVector<QString> fullPaths;
    for(const QString& path : paths) {
        if(QFileInfo(path).isSymLink())
            break;
        for(const QString &executable : executables) {
            const QString pp = path + QChar::fromLatin1('/') + executable;
            if(QFile::exists(pp)) {
                fullPaths.push_back(pp);
            }
        }
    }
    return fullPaths;
}

const QVector<QString> SysUtils::terminalApps = {
    QStringLiteral("x-terminal-emulator"),
    QStringLiteral("gnome-terminal"),
    QStringLiteral("mate-terminal"),
    QStringLiteral("konsole"),
    QStringLiteral("xfce4-terminal"),
    QStringLiteral("lxterminal"),
    QStringLiteral("alacritty"),
    QStringLiteral("urxvt")
};
