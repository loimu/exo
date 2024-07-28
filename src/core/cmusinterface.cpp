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

#include <QVector>
#include <QStringList>
#include <QProcess>
#include <QRegularExpression>

#include "sysutils.h"
#include "cmusinterface.h"


CmusInterface::CmusInterface(QObject* parent) : PlayerInterface(parent),
    cmus(new QProcess(this)),
    cli(QStringLiteral("cmus-remote"))
{
    if(SysUtils::findProcessId(QStringLiteral("cmus")) < 0)
        runPlayer();
    cmus->setProgram(cli);
    cmus->setArguments(QStringList{ QStringLiteral("-Q") });
    startTimer(1500);
    connect(cmus, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &CmusInterface::notify);
    qInfo("using cmus interface");
}

void CmusInterface::runPlayer() {
    const QVector<QString> apps = SysUtils::findFullPaths(
                QVector<QString> {
                    QStringLiteral("x-terminal-emulator"),
                    QStringLiteral("gnome-terminal"),
                    QStringLiteral("konsole"),
                    QStringLiteral("xfce4-terminal"),
                    QStringLiteral("lxterminal") });
    const QString term = apps.isEmpty() ? QStringLiteral("xterm") : apps.at(0);
    QProcess::startDetached(term, QStringList{QStringLiteral("-e"),
                                              QStringLiteral("cmus")});
}

QString CmusInterface::find(const QString& string, const QString& regexp) {
    QRegularExpression re(regexp);
    QRegularExpressionMatch match = re.match(string);
    return match.hasMatch() ? match.captured(1) : QString();
}

PState CmusInterface::updateInfo() {
    const QString info = cmus->readAllStandardOutput();
    if(info.isEmpty()) {
        track.caption.clear();
        return PState::Offline;
    }
    const QString string = find(info, QStringLiteral("status\\s(.*)"));
    if(string == QLatin1String("stopped")) {
        track.caption.clear();
        return PState::Stop;
    }
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
    const QString title = find(info, QStringLiteral("stream\\s(.*)"));
    track.caption += QLatin1String("<br />") + title;
    track.totalSec = 10*60;
    if(!title.isEmpty()) {
        const QString dash = QStringLiteral(" - ");
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
    QProcess::startDetached(cli, QStringList{QStringLiteral(__option)});\
    }

SEND_COMMAND(play, "-p")
SEND_COMMAND(pause, "-u")
SEND_COMMAND(playPause, "-u")
SEND_COMMAND(prev, "-r")
SEND_COMMAND(next, "-n")
SEND_COMMAND(stop, "-s")

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
    /* all three methods here are operating on the queue, not playlist */
    clearPlaylist();
    QProcess::startDetached(cli, QStringList() << QStringLiteral("-q") << file);
    next();
    play();
}

void CmusInterface::appendFile(const QStringList& files) {
    QProcess::startDetached(
                cli, QStringList() << QStringLiteral("-q") << files);
}

void CmusInterface::clearPlaylist() {
    /* despite the name of the method in fact it clears the player queue */
    QProcess::startDetached(
                cli, QStringList{ QStringLiteral("-q"), QStringLiteral("-c") });
}

void CmusInterface::timerEvent(QTimerEvent *event) {
    Q_UNUSED(event);
    cmus->start(QIODevice::ReadOnly);
}

void CmusInterface::shutdown() {
    cmus->close();
}
