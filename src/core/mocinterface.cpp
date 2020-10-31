/* ========================================================================
*    Copyright (C) 2013-2020 Blaze <blaze@vivaldi.net>
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

#include "config.h"

#include <QVector>
#include <QStringList>
#include <QProcess>
#include <QRegularExpression>

#include "sysutils.h"
#include "mocinterface.h"

#define OSD_OPT "OnSongChange=" INSTALL_PREFIX "/bin/moc-osd"
#define PLAYER_EXECUTABLE "mocp"


MocInterface::MocInterface(QObject* parent) : PlayerInterface(parent),
    moc(new QProcess(this)),
    player(QStringLiteral(PLAYER_EXECUTABLE))
{
    if(SysUtils::findProcessId(player) < 0)  // check if player is running
        QProcess::startDetached(
                    player,
                    QStringList{QStringLiteral("-SO"),QStringLiteral(OSD_OPT)});
    moc->setProgram(player);
    moc->setArguments(QStringList { QStringLiteral("-Q"), QStringLiteral(
                                    "%state{a}%a{t}%t{A}%A{f}%file{tt}%tt"
                                    "{ts}%ts{cs}%cs{T}%title") });
    startTimer(1000);
    connect(moc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MocInterface::notify);
}

void MocInterface::runServer() {
    if(SysUtils::findProcessId(player) < 0) {  // check if player is running
        QProcess p;
        p.start(player,
                QStringList{ QStringLiteral("-SO"), QStringLiteral(OSD_OPT) },
                QIODevice::NotOpen);
        p.waitForFinished();
    }
}

PState MocInterface::updateInfo() {
    const QString info = moc->readAllStandardOutput();
    if(info.isEmpty())
        return PState::Offline;
    if(info.startsWith(QLatin1String("STOP")))
        return PState::Stop;
    static const QRegularExpression re(
                QStringLiteral("^(.*)\\{a\\}(.*)\\{t\\}(.*)\\{A\\}(.*)"
                               "\\{f\\}(.*)\\{tt\\}(.*)\\{ts\\}(.*)"
                               "\\{cs\\}(.*)\\{T\\}(.*)\n"));
    QRegularExpressionMatch match = re.match(info);
    PState state = PState::Offline;
    if(match.captured(1) == QLatin1String("PLAY"))
        state = PState::Play;
    else if(match.captured(1) == QLatin1String("PAUSE"))
        state = PState::Pause;
    track.artist = match.captured(2);
    track.title = match.captured(3);
    track.album = match.captured(4);
    track.file = match.captured(5);
    track.totalTime = match.captured(6);
    track.totalSec = match.captured(7).toInt();
    track.currSec = match.captured(8).toInt();
    track.caption = match.captured(9);
    track.isStream = track.totalTime.isEmpty();
    if(track.caption.isEmpty())
        track.caption = track.file;
    if(track.isStream) {
        track.totalSec = 8*60;
        const QString dash = QStringLiteral(" - ");
        if(track.caption.contains(dash)) {
            track.artist = track.caption.section(dash, 0, 0);
            track.title = track.caption.section(dash, 1, -1);
        }
    }
    return state;
}

const QString MocInterface::id() const {
    return QStringLiteral("music on console");
}

#define SEND_COMMAND(__method, __option)\
    void MocInterface::__method() {\
    QProcess::startDetached(player, QStringList{QStringLiteral(__option)});\
    }

SEND_COMMAND(play, "-p")
SEND_COMMAND(pause,"-P")
SEND_COMMAND(playPause, "-G")
SEND_COMMAND(prev, "-r")
SEND_COMMAND(next, "-f")
SEND_COMMAND(stop, "-s")
SEND_COMMAND(quit, "-x")
SEND_COMMAND(clearPlaylist, "-c")

#define SEND_COMMAND_PARAM(__method, __option)\
    void MocInterface::__method(int param) {\
    QProcess::startDetached(player,\
    QStringList() << QString(QStringLiteral(__option)).arg(param));\
    }

SEND_COMMAND_PARAM(jump, "-j%1s")
SEND_COMMAND_PARAM(seek, "-k%1")
SEND_COMMAND_PARAM(volume, "-v%1")
SEND_COMMAND_PARAM(changeVolume, "-v+%1")

void MocInterface::showPlayer() {
    const QVector<QString> apps = SysUtils::findFullPaths(
                QVector<QString> {
                    QStringLiteral("x-terminal-emulator"),
                    QStringLiteral("gnome-terminal"),
                    QStringLiteral("konsole"),
                    QStringLiteral("xfce4-terminal"),
                    QStringLiteral("lxterminal") });
    const QString term = apps.isEmpty() ? QStringLiteral("xterm") : apps.at(0);
    QProcess::startDetached(
                term,
                QStringList{QStringLiteral("-e"),
                            QStringLiteral(PLAYER_EXECUTABLE " -O " OSD_OPT)});
}

void MocInterface::openUri(const QString& file) {
    runServer();
    QProcess::startDetached(player,
                            QStringList() << QStringLiteral("-l") << file);
}

void MocInterface::appendFile(const QStringList& files) {
    runServer();
    QProcess::startDetached(player,
                            QStringList() << QStringLiteral("-a") << files);
}

void MocInterface::timerEvent(QTimerEvent* event) {
    Q_UNUSED(event);
    moc->start(QIODevice::ReadOnly);
}

void MocInterface::shutdown() {
    moc->close();
}
