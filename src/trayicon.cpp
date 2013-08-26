/* ========================================================================
*    Copyright (C) 2013 Blaze <blaze@jabster.pl>
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

#include <QtGui>

#include "trayicon.h"
#include "playerinterface.h"
#include "lyricswindow.h"
#include "aboutdialog.h"

TrayIcon::TrayIcon(PlayerInterface *player) {
    m_player = player;

    createActions();
    createTrayIcon();

    trayIcon->show();

    connect(m_player, SIGNAL(timerSignal()), this, SLOT(updateToolTip()));
}

void TrayIcon::createActions() {
    lyricsAction = new QAction(tr("&Lyrics"), this);
    connect(lyricsAction, SIGNAL(triggered()), this, SLOT(showLyricsWindow()));
    playAction = new QAction(tr("&Play"), this);
    connect(playAction, SIGNAL(triggered()), m_player, SLOT(play()));
    QIcon playIcon(":/images/play.png");
    playAction->setIcon(playIcon);
    pauseAction = new QAction(tr("P&ause"), this);
    connect(pauseAction, SIGNAL(triggered()), m_player, SLOT(pause()));
    QIcon pauseIcon(":/images/pause.png");
    pauseAction->setIcon(pauseIcon);
    prevAction = new QAction(tr("P&rev"), this);
    connect(prevAction, SIGNAL(triggered()), m_player, SLOT(prev()));
    QIcon prevIcon(":/images/prev.png");
    prevAction->setIcon(prevIcon);
    nextAction = new QAction(tr("&Next"), this);
    connect(nextAction, SIGNAL(triggered()), m_player, SLOT(next()));
    QIcon nextIcon(":/images/next.png");
    nextAction->setIcon(nextIcon);
    stopAction = new QAction(tr("&Stop"), this);
    connect(stopAction, SIGNAL(triggered()), m_player, SLOT(stop()));
    QIcon stopIcon(":/images/stop.png");
    stopAction->setIcon(stopIcon);
    aboutAction = new QAction(tr("A&bout"), this);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), m_player, SLOT(quit()));
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    QIcon quitIcon(":/images/close.png");
    quitAction->setIcon(quitIcon);
}

void TrayIcon::createTrayIcon() {
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(lyricsAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(playAction);
    trayIconMenu->addAction(pauseAction);
    trayIconMenu->addAction(prevAction);
    trayIconMenu->addAction(nextAction);
    trayIconMenu->addAction(stopAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(aboutAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    QIcon icon(":/images/22.png");
    trayIcon->setIcon(icon);

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                      SLOT(clicked(QSystemTrayIcon::ActivationReason)));
}

void TrayIcon::clicked(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
        case QSystemTrayIcon::Context:
            if(m_about)
                aboutAction->setEnabled(false);
            else
                aboutAction->setEnabled(true);
            break;
        case QSystemTrayIcon::DoubleClick:
            m_player->openWindow();
            break;
        case QSystemTrayIcon::Trigger:
            break;
        case QSystemTrayIcon::MiddleClick:
            m_player->pause();
            break;
        default:
            break;
    }
}

void TrayIcon::updateToolTip() {
    QStringList info = m_player->m_list;
    QString tooltip = tr("<html><b>Stopped</b>");
    if(m_player->isServerRunning() && info.size() > 0) {
        if(info.at(0) != "STOP") {
            tooltip = QString("<html><b>%1</b>").arg(info.at(2));
            if(!info.at(1).startsWith("http")) {
                tooltip.append(tr("<br />Current Time: %1/%2").arg(info.at(9))
                               .arg(info.at(6)));
                tooltip.append(QString("<br /><img src='%1' width='300' />")
                               .arg(coverPath()));
            }
        }
        tooltip.append("</html>");
    }
    else
        tooltip = tr("Player is not running, make a doubleclick.");
    trayIcon->setToolTip(tooltip);
}

void TrayIcon::showLyricsWindow() {
    LyricsWindow *lyricsWindow = new LyricsWindow(this);
    lyricsWindow->show();
}

void TrayIcon::showAboutDialog() {
    m_about = new AboutDialog(this);
    m_about->show();
}

QString TrayIcon::coverPath() {
    QString repl = m_player->m_list.at(1);
    QString path = repl.replace(QRegExp("(.*)/(.*)"), "\\1");
    QDir dir(path);
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg";
    dir.setNameFilters(filters);
    if(dir.entryList().size() > 0)
        return path + "/" + dir.entryList().at(0);
    else
        return ":/images/nocover.png";
}
