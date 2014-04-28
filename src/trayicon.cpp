/* ========================================================================
*    Copyright (C) 2013-2014 Blaze <blaze@jabster.pl>
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

#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QWheelEvent>
#include <QPointer>
#include <QDir>
#include <QSettings>
#include <QFileDialog>

#include "exo.h"
#include "playerinterface.h"
#include "lyricsdialog.h"
#include "aboutdialog.h"
#include "scrobbler.h"
#include "trayicon.h"

TrayIcon::TrayIcon() {
    PlayerInterface* player = PlayerInterface::instance();
    createActions();
    createTrayIcon();
    trayIcon->show();
    connect(player, SIGNAL(updateStatus(QString, QString, QString, QString)),
            this, SLOT(updateToolTip(QString, QString, QString, QString)));
    connect(this, SIGNAL(playerOpenWindow()), player, SLOT(openWindow()));
    connect(this, SIGNAL(playerTogglePause()), player, SLOT(pause()));
    connect(this, SIGNAL(playerVolumeDown()), player, SLOT(vold()));
    connect(this, SIGNAL(playerVolumeUp()), player, SLOT(volu()));
}

void TrayIcon::createActions() {
    PlayerInterface* player = PlayerInterface::instance();
    filesAction = new QAction(tr("A&dd ..."), this);
    connect(filesAction, SIGNAL(triggered()), this, SLOT(addFiles()));
    lyricsAction = new QAction(tr("&Lyrics"), this);
    connect(lyricsAction, SIGNAL(triggered()), this, SLOT(showLyricsWindow()));
    playAction = new QAction(tr("&Play"), this);
    connect(playAction, SIGNAL(triggered()), player, SLOT(play()));
    QIcon playIcon(":/images/play.png");
    playAction->setIcon(playIcon);
    pauseAction = new QAction(tr("P&ause"), this);
    connect(pauseAction, SIGNAL(triggered()), player, SLOT(pause()));
    QIcon pauseIcon(":/images/pause.png");
    pauseAction->setIcon(pauseIcon);
    prevAction = new QAction(tr("P&rev"), this);
    connect(prevAction, SIGNAL(triggered()), player, SLOT(prev()));
    QIcon prevIcon(":/images/prev.png");
    prevAction->setIcon(prevIcon);
    nextAction = new QAction(tr("&Next"), this);
    connect(nextAction, SIGNAL(triggered()), player, SLOT(next()));
    QIcon nextIcon(":/images/next.png");
    nextAction->setIcon(nextIcon);
    stopAction = new QAction(tr("&Stop"), this);
    connect(stopAction, SIGNAL(triggered()), player, SLOT(stop()));
    QIcon stopIcon(":/images/stop.png");
    stopAction->setIcon(stopIcon);
    aboutAction = new QAction(tr("A&bout"), this);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), player, SLOT(quit()));
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    QIcon quitIcon(":/images/close.png");
    quitAction->setIcon(quitIcon);
    setQuitBehaviourAction = new QAction(tr("Close moc on exit"), this);
    setQuitBehaviourAction->setCheckable(true);
    connect(setQuitBehaviourAction, SIGNAL(triggered()),
            this, SLOT(setQuitBehaviour()));
    setScrobblingAction = new QAction(tr("Enable scrobbling"), this);
    setScrobblingAction->setCheckable(true);
    connect(setScrobblingAction, SIGNAL(triggered()),
            this, SLOT(setScrobbling()));
}

void TrayIcon::createTrayIcon() {
    trayIconMenu = new QMenu(this);
    settingsMenu = new QMenu(trayIconMenu);
    settingsMenu->setTitle(tr("Settings"));
    trayIconMenu->addAction(filesAction);
    trayIconMenu->addAction(lyricsAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(playAction);
    trayIconMenu->addAction(pauseAction);
    trayIconMenu->addAction(prevAction);
    trayIconMenu->addAction(nextAction);
    trayIconMenu->addAction(stopAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(settingsMenu->menuAction());
    settingsMenu->addAction(setQuitBehaviourAction);
    settingsMenu->addAction(setScrobblingAction);
    trayIconMenu->addAction(aboutAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    QIcon icon(":/images/32.png");
    trayIcon->setIcon(icon);
    trayIcon->installEventFilter(this);
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                      SLOT(clicked(QSystemTrayIcon::ActivationReason)));
}

void TrayIcon::clicked(QSystemTrayIcon::ActivationReason reason) {
    QSettings* settings = Exo::app()->settings();
    switch (reason) {
        case QSystemTrayIcon::Context:
            if(m_about)
                aboutAction->setEnabled(false);
            else
                aboutAction->setEnabled(true);
            if(!settings->value("scrobbler/disabled").toBool())
                setScrobblingAction->setChecked(true);
            if(settings->value("player/quit").toBool())
                setQuitBehaviourAction->setChecked(true);
            break;
        case QSystemTrayIcon::DoubleClick:
            emit playerOpenWindow();
            break;
        case QSystemTrayIcon::Trigger:
            break;
        case QSystemTrayIcon::MiddleClick:
            emit playerTogglePause();
            break;
        default:
            break;
    }
}

bool TrayIcon::eventFilter(QObject* object, QEvent* event) {
    if (QObject::eventFilter(object, event))
        return true;
    if (object != trayIcon)
        return false;
    if (event->type() == QEvent::Wheel) {
        QWheelEvent* e = static_cast<QWheelEvent*>(event);
        if (e->delta() < 0)
            emit playerVolumeDown();
        else
            emit playerVolumeUp();
        return true;
    }
    return false;
}

void TrayIcon::updateToolTip(QString message, QString currentTime,
                             QString totalTime, QString path) {
    QString tooltip = QString("<table width=\"300\"><tr><td><b>%1</b>"
                              "</td></tr></table>").arg(message);
    // NOTE: path variable should be empty if radio stream is playing
    if(!path.isEmpty()) {
        tooltip.append(QString("<br />Current time: %1/%2<br />"
                               "<img src=\"%3\" width=\"300\" />")
                       .arg(currentTime)
                       .arg(totalTime)
                       .arg(coverPath(path)));
    }
    trayIcon->setToolTip(tooltip);
}

QString TrayIcon::coverPath(QString path) {
    path.replace(QRegExp("(.*)/(.*)"), "\\1");
    QDir dir(path);
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg";
    dir.setNameFilters(filters);
    if(dir.entryList().size() > 0)
        return path + "/" + dir.entryList().at(0);
    else
        return ":/images/nocover.png";
}

void TrayIcon::showLyricsWindow() {
    QPointer<LyricsDialog> lyricsDialog = new LyricsDialog(this);
    lyricsDialog->show();
}

void TrayIcon::showAboutDialog() {
    m_about = new AboutDialog(this);
    m_about->show();
}

void TrayIcon::setQuitBehaviour() {
    QSettings* settings = Exo::app()->settings();
    if(setQuitBehaviourAction->isChecked())
        settings->setValue("player/quit", true);
    else
        settings->setValue("player/quit", false);
}

void TrayIcon::setScrobbling() {
    if(setScrobblingAction->isChecked())
        emit loadScrobbler();
    else
        emit unloadScrobbler();
}

void TrayIcon::addFiles() {
    QStringList files = QFileDialog::getOpenFileNames(this,
                                                     "Add files to playlist",
                                                     "",
                                                     "Media (*.ogg *.mp3 *.flac)");
    QStringList::Iterator it = files.begin();
    while(it != files.end()) {
        PlayerInterface::instance()->appendFile(*it);
        ++it;
    }
}
