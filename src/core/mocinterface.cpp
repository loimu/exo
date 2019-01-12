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

#include "config.h"

#include <QStringList>
#include <QProcess>
#include <QRegularExpression>

#include "mocinterface.h"

#define OSD_OPT "OnSongChange=" INSTALL_PREFIX "/bin/moc-osd"
#define PLAYER_EXECUTABLE "mocp"


MocInterface::MocInterface(QObject* parent) : PlayerInterface(parent),
    moc(new QProcess(this)),
    player(QStringLiteral(PLAYER_EXECUTABLE))
{
    if(!isPlayerRunning(player))
        QProcess::startDetached(player,
                                QStringList{QStringLiteral("-SO"),
                                            QStringLiteral(OSD_OPT)});
    startTimer(1000);
    connect(moc, QOverload<int>::of(&QProcess::finished),
            this, &MocInterface::notify);
}

MocInterface::~MocInterface() {
    moc->close();
}

PState MocInterface::updateInfo() {
    QString info = moc->readAllStandardOutput();
    if(info.isEmpty())
        return PState::Offline;
    if(info.startsWith(QLatin1String("STOP")))
        return PState::Stop;
    QRegularExpression re(
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
        QString dash = QStringLiteral(" - ");
        if(track.caption.contains(dash)) {
            track.artist = track.caption.section(dash, 0, 0);
            track.title = track.caption.section(dash, 1, -1);
        }
    }
    return state;
}

QString MocInterface::id() const {
    return QStringLiteral("music on console");
}

#define SEND_COMMAND(__method, __option)\
    void MocInterface::__method() {\
    QProcess::startDetached(player, QStringList{__option});\
    }

SEND_COMMAND(play, QStringLiteral("-p"))
SEND_COMMAND(pause,QStringLiteral("-P"))
SEND_COMMAND(playPause, QStringLiteral("-G"))
SEND_COMMAND(prev, QStringLiteral("-r"))
SEND_COMMAND(next, QStringLiteral("-f"))
SEND_COMMAND(stop, QStringLiteral("-s"))
SEND_COMMAND(quit, QStringLiteral("-x"))
SEND_COMMAND(clearPlaylist, QStringLiteral("-c"))

#define SEND_COMMAND_PARAM(__method, __option)\
    void MocInterface::__method(int param) {\
    QProcess::startDetached(player,\
    QStringList() << QString(__option).arg(param));\
    }

SEND_COMMAND_PARAM(jump, QStringLiteral("-j%1s"))
SEND_COMMAND_PARAM(seek, QStringLiteral("-k%1"))
SEND_COMMAND_PARAM(volume, QStringLiteral("-v%1"))
SEND_COMMAND_PARAM(changeVolume, QStringLiteral("-v+%1"))

void MocInterface::showPlayer() {
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
    QProcess::startDetached(
                term,
                QStringList{QStringLiteral("-e"),
                            QStringLiteral(PLAYER_EXECUTABLE " -O " OSD_OPT)});
}

void MocInterface::openUri(const QString& file) {
    if(!isPlayerRunning(player)) {
        QProcess procPlayer;
        procPlayer.start(player, QStringList{QStringLiteral("-SO"),
                                             QStringLiteral(OSD_OPT)});
        procPlayer.waitForFinished();
    }
    QProcess::startDetached(player,
                            QStringList() << QStringLiteral("-l") << file);
}

void MocInterface::appendFile(const QStringList& files) {
    if(!isPlayerRunning(player)) {
        QProcess procPlayer;
        procPlayer.start(player, QStringList{QStringLiteral("-SO"),
                                             QStringLiteral(OSD_OPT)});
        procPlayer.waitForFinished();
    }
    QProcess::startDetached(player,
                            QStringList() << QStringLiteral("-a") << files);
}

void MocInterface::timerEvent(QTimerEvent* event) {
    Q_UNUSED(event);
    moc->start(player, QStringList{
                   QStringLiteral("-Q"),
                   QStringLiteral("%state{a}%a{t}%t{A}%A{f}%file{tt}%tt{ts}%ts"
                   "{cs}%cs{T}%title")});
}
