/* ========================================================================
*    Copyright (C) 2013-2019 Blaze <blaze@vivaldi.net>
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
#include <QProcess>
#include <QRegularExpression>

#include "cmusinterface.h"


CmusInterface::CmusInterface(QObject* parent) : PlayerInterface(parent),
    cmus(new QProcess(this)),
    cli(QStringLiteral("cmus-remote"))
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
    QProcess::startDetached(term, QStringList{QStringLiteral("-e"),
                                              QStringLiteral("cmus")});
}

QString CmusInterface::find(const QString& string, const QString& regexp) {
    QRegularExpression re(regexp);
    QRegularExpressionMatch match = re.match(string);
    return match.hasMatch() ? match.captured(1) : QString();
}

PState CmusInterface::updateInfo() {
    QString info = cmus->readAllStandardOutput();
    if(info.isEmpty())
        return PState::Offline;
    QString string = find(info, QStringLiteral("status\\s(.*)"));
    if(string == QLatin1String("stopped"))
        return PState::Stop;
    PState state = PState::Offline;
    if(string == QLatin1String("playing"))
        state = PState::Play;
    if(string == QLatin1String("paused"))
        state = PState::Pause;
    track.artist = find(info, QStringLiteral("tag\\sartist\\s(.*)"));
    track.title = find(info, QStringLiteral("tag\\stitle\\s(.*)"));
    track.album = find(info, QStringLiteral("tag\\salbum\\s(.*)"));
    track.file = find(info, QStringLiteral("file\\s(.*)"));
    track.totalSec = find(info, QStringLiteral("duration\\s(.*)")).toInt();
    track.currSec = find(info, QStringLiteral("position\\s(.*)")).toInt();
    track.totalTime = QString(QStringLiteral("%1:%2")).arg(track.totalSec / 60)
            .arg(track.totalSec % 60, 2, 10, QChar::fromLatin1('0'));
    track.caption = track.artist.isEmpty()
            ? track.title
            : QString(QStringLiteral("%1 - %2 (%3)")).arg(
                  track.artist, track.title, track.album);
    track.isStream = !track.file.startsWith(QChar::fromLatin1('/'));
    if(!track.isStream)
        return state;
    QString title = find(info, QStringLiteral("stream\\s(.*)"));
    track.caption += QLatin1String("<br />") + title;
    track.totalSec = 8*60;
    if(!title.isEmpty()) {
        QString dash = QStringLiteral(" - ");
        if(title.contains(dash)) {
            track.artist = title.section(dash, 0, 0);
            track.title = title.section(dash, 1, -1);
        }
    }
    return state;
}

const QString CmusInterface::id() const {
    return QStringLiteral("Cmus");
}

#define SEND_COMMAND(__method, __option)\
    void CmusInterface::__method() {\
    QProcess::startDetached(cli, QStringList{__option});\
    }

SEND_COMMAND(play, QStringLiteral("-p"))
SEND_COMMAND(pause, QStringLiteral("-u"))
SEND_COMMAND(playPause, QStringLiteral("-u"))
SEND_COMMAND(prev, QStringLiteral("-r"))
SEND_COMMAND(next, QStringLiteral("-n"))
SEND_COMMAND(stop, QStringLiteral("-s"))

void CmusInterface::quit() {
    QProcess::startDetached(cli, QStringList{QStringLiteral("-C"),
                                             QStringLiteral("quit")});
}

void CmusInterface::jump(int pos) {
    QProcess::startDetached(cli, QStringList() << QStringLiteral("-k")
                            << QString::number(pos));
}

void CmusInterface::seek(int offset) {
    QString o = (offset > 0) ? QLatin1String("+") + QString::number(offset)
                             : QString::number(offset);
    QProcess::startDetached(cli, QStringList() << QStringLiteral("-k") << o);
}

void CmusInterface::volume(int lev) {
    QProcess::startDetached(cli, QStringList() << QStringLiteral("-v")
                            << QString::number(lev) + QLatin1String("%"));
}

void CmusInterface::changeVolume(int delta) {
    QString d = ((delta > 0) ? QLatin1String("+") + QString::number(delta)
                             : QString::number(delta)) + QLatin1String("%");
    QProcess::startDetached(cli, QStringList() << QStringLiteral("-v") << d);
}

void CmusInterface::showPlayer() {
    runPlayer();
}

void CmusInterface::openUri(const QString& file) {
    QProcess::startDetached(cli, QStringList() << QStringLiteral("-q")
                            << QStringLiteral("-c")); // clear queue
    QProcess::startDetached(cli, QStringList() << QStringLiteral("-q")
                            << file); // append file to queue
    QProcess::startDetached(cli, QStringList() << QStringLiteral("-n"));
}

void CmusInterface::appendFile(const QStringList& files) {
    QProcess::startDetached(
                cli, QStringList() << QStringLiteral("-q") << files);
}

void CmusInterface::clearPlaylist() {
    /* despite the name of the method in fact it clears the player queue */
    QProcess::startDetached(cli, QStringList() << QStringLiteral("-q")
                            << QStringLiteral("-c"));
}

void CmusInterface::timerEvent(QTimerEvent *event) {
    Q_UNUSED(event);
    cmus->close();
    cmus->start(cli, QStringList{QStringLiteral("-Q")});
}

void CmusInterface::shutdown() {
    cmus->close();
}
