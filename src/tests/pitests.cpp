/* ========================================================================
*    Copyright (C) 2013-2018 Blaze <blaze@vivaldi.net>
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

#include "playerinterface.h"
#include "mocinterface.h"
#include "cmusinterface.h"
#include "pitests.h"


void PITests::initTestCase() {
    player = new MocInterface(this);
    QTest::qWait(2000);
    player->appendFile(QStringList{
                           QStringLiteral(SOURCE_ROOT "/test/audiofile1.ogg"),
                           QStringLiteral(SOURCE_ROOT "/test/audiofile2.ogg")});
    QTest::qWait(2000);
}

void PITests::testPlayback() {
    QString tokenString1 = QStringLiteral("Audiofile 1");
    QString tokenString2 = QStringLiteral("Audiofile 2");
    player->play();
    QTest::qWait(1000);
    QString t1 = player->trackObject()->title;
    player->next();
    QTest::qWait(1000);
    QString t2 = player->trackObject()->title;
    player->prev();
    QTest::qWait(1000);
    player->pause();
    QTest::qWait(1000);
    QString t3 = player->trackObject()->title;
    player->stop();
    QCOMPARE(tokenString1, t1);
    QCOMPARE(tokenString2, t2);
    QCOMPARE(tokenString1, t3);
}

void PITests::cleanupTestCase() {
    QTest::qWait(1000);
    player->quit();
}
