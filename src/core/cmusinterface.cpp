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

#include <QStringList>
#include <QTime>
#include <QProcess>

#include "cmusinterface.h"

#define PLAYER_EXECUTABLE "cmus-remote"

CmusInterface::CmusInterface(QObject* parent) : PlayerInterface(parent),
    cmus(new QProcess(this)),
    cli(QStringLiteral(PLAYER_EXECUTABLE))
{
    QProcess proc;
    proc.start(QStringLiteral("pidof"), QStringList{QStringLiteral("cmus")});
    proc.waitForFinished();
    if(QString::fromUtf8(proc.readAllStandardOutput()).isEmpty())
        runPlayer();
    startTimer(1000);
    connect(cmus, QOverload<int>::of(&QProcess::finished),
            this, &CmusInterface::notify);
}

CmusInterface::~CmusInterface() {
    cmus->close();
}

void CmusInterface::runPlayer() {
    QString term = QStringLiteral("xterm"); // xterm is a fallback app
    QStringList apps = Process::detect(
                QStringList{
                    QStringLiteral("x-terminal-emulator"),
                    QStringLiteral("gnome-terminal"),
                    QStringLiteral("konsole"),
                    QStringLiteral("xfce4-terminal"),
                    QStringLiteral("lxterminal")});
    if(!apps.isEmpty())
        term = apps.at(0);
    Process::execute(term, QStringList{QStringLiteral("-e"),
                                       QStringLiteral("cmus")});
}

QString CmusInterface::find(const QString& string, const QString& regEx) {
    QRegExp findRgx(regEx);
    findRgx.setMinimal(true);
    findRgx.indexIn(string);
    return findRgx.cap(1);
}

PIState CmusInterface::updateInfo() {
    QString info = cmus->readAllStandardOutput();
    if(info.isEmpty())
        return PIState::Offline;
    QString string = find(info, QStringLiteral("status\\s(.*)\\n"));
    if(string == QLatin1String("stopped"))
        return PIState::Stop;
    PIState state = PIState::Offline;
    if(string == QLatin1String("playing"))
        state = PIState::Play;
    if(string == QLatin1String("paused"))
        state = PIState::Pause;
    track.artist = find(info, QStringLiteral("tag\\sartist\\s(.*)\\n"));
    track.title = find(info, QStringLiteral("tag\\stitle\\s(.*)\\n"));
    track.album = find(info, QStringLiteral("tag\\salbum\\s(.*)\\n"));
    track.file = find(info, QStringLiteral("file\\s(.*)\\n"));
    track.totalSec = find(info, QStringLiteral("duration\\s(.*)\\n")).toInt();
    track.currSec = find(info, QStringLiteral("position\\s(.*)\\n")).toInt();
    track.totalTime = QTime().addSecs(
                track.totalSec).toString(QStringLiteral("mm:ss"));
    track.caption = track.artist.isEmpty()
            ? track.title
            : track.artist + QLatin1String(" - ") + track.title;
    track.isStream = !track.file.startsWith(QChar::fromLatin1('/'));
    if(!track.isStream)
        return state;
    QString title = find(info, QStringLiteral("stream\\s(.*)\\n"));
    track.caption += QLatin1String("<br />") + title;
    track.totalSec = 8*60;
    if(!title.isEmpty()) {
        QRegExp artistRgx(QStringLiteral("^(.*)\\s-\\s"));
        artistRgx.setMinimal(true);
        artistRgx.indexIn(title);
        track.artist = artistRgx.cap(1);
        QRegExp titleRgx(QStringLiteral("\\s-\\s(.*)$"));
        titleRgx.indexIn(title);
        track.title = titleRgx.cap(1);
    }
    return state;
}

QString CmusInterface::id() {
    return QStringLiteral("Cmus");
}

#define SEND_COMMAND(__method, __option)\
    bool CmusInterface::__method() {\
    return Process::execute(cli, QStringList{__option});\
    }

SEND_COMMAND(play, QStringLiteral("-p"))
SEND_COMMAND(pause, QStringLiteral("-u"))
SEND_COMMAND(playPause, QStringLiteral("-u"))
SEND_COMMAND(prev, QStringLiteral("-r"))
SEND_COMMAND(next, QStringLiteral("-n"))
SEND_COMMAND(stop, QStringLiteral("-s"))

bool CmusInterface::quit() {
    return Process::execute(cli, QStringList{QStringLiteral("-C"),
                                             QStringLiteral("quit")});
}

bool CmusInterface::jump(int pos) {
    return Process::execute(cli, QStringList() << QStringLiteral("-k")
                            << QString::number(pos));
}

bool CmusInterface::seek(int offset) {
    QString o = (offset > 0) ? QLatin1String("+") + QString::number(offset)
                             : QString::number(offset);
    return Process::execute(cli, QStringList() << QStringLiteral("-k") << o);
}

bool CmusInterface::volume(int lev) {
    return Process::execute(cli, QStringList() << QStringLiteral("-v")
                            << QString::number(lev) + QLatin1String("%"));
}

bool CmusInterface::changeVolume(int delta) {
    QString d = ((delta > 0) ? QLatin1String("+") + QString::number(delta)
                             : QString::number(delta)) + QLatin1String("%");
    return Process::execute(cli, QStringList() << QStringLiteral("-v") << d);
}

void CmusInterface::showPlayer() {
    runPlayer();
}

bool CmusInterface::openUri(const QString& file) {
    return Process::execute(cli, QStringList() << QStringLiteral("-q") << file);
}

bool CmusInterface::appendFile(const QStringList& files) {
    return Process::execute(cli,
                            QStringList() << QStringLiteral("-q") << files);
}

void CmusInterface::timerEvent(QTimerEvent *event) {
    Q_UNUSED(event);
    cmus->start(cli, QStringList{QStringLiteral("-Q")});
}
