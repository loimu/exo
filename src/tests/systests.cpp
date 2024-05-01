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

#include <QTest>
#include <QProcess>

#include "sysutils.h"
#include "systests.h"


void SysTests::findFullPathsTest() {
    auto ret = SysUtils::findFullPaths(QVector<QString>{QStringLiteral("bash")});

    QCOMPARE(ret.at(0), "/usr/bin/bash");
}

void SysTests::findProcessIdTest() {
    QProcess process;
    process.start("sleep", QStringList{"3"});
    int expected = process.processId();
    int actual = SysUtils::findProcessId("sleep");
    process.close();

    QCOMPARE(expected, actual);
}
