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

#include <sys/file.h>

#include <QCoreApplication>
#include <QFile>
#include <QDir>

#include "singleinstance.h"


SingleInstance::SingleInstance()
    : lockFile(new QFile(QDir::tempPath() + QLatin1Char('/')
                         + qApp->applicationName() + QLatin1String(".lock")))
{
    lockFile->open(QIODevice::ReadWrite);
    if(flock(lockFile->handle(), LOCK_EX | LOCK_NB) == -1)
        success = false;
}

SingleInstance::~SingleInstance() {
    flock(lockFile->handle(), LOCK_UN);
    lockFile->close();
}
