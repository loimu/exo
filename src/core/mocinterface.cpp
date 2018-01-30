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

#include "config.h"

#include <QStringList>
#include <QProcess>

#include "mocinterface.h"

#define OSD_OPT "OnSongChange=" INSTALL_PREFIX "/bin/moc-osd"
#define PLAYER_CLI_EXECUTABLE "mocp"

MocInterface::MocInterface(QObject* parent) : PlayerInterface(parent),
    player(QStringLiteral(PLAYER_CLI_EXECUTABLE)),
    moc(new QProcess())
{
    if(!isServerRunning())
        runServer();
    startTimer(1000);

    connect(moc, QOverload<int>::of(&QProcess::finished), this, [=] {
        QString info = moc->readAllStandardOutput();
        if(info.isEmpty()) {
            notify(PIState::Offline);
            return;
        }
        if(info.startsWith(QLatin1String("STOP"))) {
            notify(PIState::Stop);
            return;
        }
        QRegExp infoRgx(
                    QStringLiteral("^(.*)\\{a\\}(.*)\\{t\\}(.*)\\{A\\}(.*)"
                                   "\\{f\\}(.*)\\{tt\\}(.*)\\{ts\\}(.*)"
                                   "\\{cs\\}(.*)\\{T\\}(.*)\n"));
        infoRgx.setMinimal(true);
        infoRgx.indexIn(info);
        PIState state = PIState::Offline;
        if(infoRgx.cap(1) == QLatin1String("PLAY"))
            state = PIState::Play;
        else if(infoRgx.cap(1) == QLatin1String("PAUSE"))
            state = PIState::Pause;
        track.artist = infoRgx.cap(2);
        track.title = infoRgx.cap(3);
        track.album = infoRgx.cap(4);
        track.file = infoRgx.cap(5);
        track.totalTime = infoRgx.cap(6);
        track.totalSec = infoRgx.cap(7).toInt();
        track.currSec = infoRgx.cap(8).toInt();
        track.caption = infoRgx.cap(9);
        track.isStream = track.totalTime.isEmpty();
        if(track.caption.isEmpty())
            track.caption = track.file;
        if(track.isStream) {
            track.totalSec = 8*60;
            QRegExp artistRgx(QStringLiteral("^(.*)\\s-\\s"));
            artistRgx.setMinimal(true);
            artistRgx.indexIn(track.caption);
            track.artist = artistRgx.cap(1);
            QRegExp titleRgx(QStringLiteral("\\s-\\s(.*)$"));
            titleRgx.indexIn(track.caption);
            track.title = titleRgx.cap(1);
        }
        notify(state);
    });
}

QString MocInterface::id() {
    return QLatin1String("music on console");
}

bool MocInterface::isServerRunning() {
    return !Process::getOutput(
                QStringLiteral("pidof"),
                QStringList{player}).isEmpty();
}

bool MocInterface::runServer() {
    return Process::execute(
                player,
                QStringList{QStringLiteral("-SO"), QStringLiteral(OSD_OPT)});
}

#define SEND_COMMAND(__method, __option)\
    bool MocInterface::__method() {\
    return Process::execute(player, QStringList{__option});\
    }

SEND_COMMAND(play, QStringLiteral("-p"))
SEND_COMMAND(pause,QStringLiteral("-P"))
SEND_COMMAND(playPause, QStringLiteral("-G"))
SEND_COMMAND(prev, QStringLiteral("-r"))
SEND_COMMAND(next, QStringLiteral("-f"))
SEND_COMMAND(stop, QStringLiteral("-s"))
SEND_COMMAND(quit, QStringLiteral("-x"))

#define SEND_COMMAND_PARAM(__method, __option)\
    bool MocInterface::__method(int param) {\
    return Process::execute(player,\
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
    Process::execute(term, QStringList{
                         QStringLiteral("-e"),
                         QStringLiteral(PLAYER_CLI_EXECUTABLE " -O " OSD_OPT)});
}

bool MocInterface::openUri(const QString& file) {
    return Process::execute(player,
                            QStringList() << QStringLiteral("-l") << file);
}

bool MocInterface::appendFile(const QStringList& files) {
    return Process::execute(player,
                            QStringList() << QStringLiteral("-a") << files);
}

void MocInterface::timerEvent(QTimerEvent* event) {
    moc->start(player, QStringList{
                   QStringLiteral("-Q"),
                   QStringLiteral("%state{a}%a{t}%t{A}%A{f}%file{tt}%tt{ts}%ts"
                   "{cs}%cs{T}%title")});
}
