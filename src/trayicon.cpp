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
#include "scrobblersettings.h"

TrayIcon::TrayIcon(PlayerInterface *player, QSettings *settings) {
    m_player = player;
    m_settings = settings;

    if(!m_settings->value("scrobbler/configured").toBool())
        showConfigurationDialog();

    createActions();
    createTrayIcon();

    trayIcon->show();

    connect(m_player, SIGNAL(updateStatus()), this, SLOT(updateToolTip()));
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

    if(m_settings->value("player/quitmoc").toBool()) {
        connect(quitAction, SIGNAL(triggered()), m_player, SLOT(quit()));
        setQuitBehaviourAction->setChecked(true);
    }
    if(m_settings->value("scrobbler/enabled").toBool()) {
        setScrobblingAction->setChecked(true);
    }
}

void TrayIcon::createTrayIcon() {
    trayIconMenu = new QMenu(this);
    settingsMenu = new QMenu(trayIconMenu);
    settingsMenu->setTitle(tr("Settings"));
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
            tooltip = "<table width=\"300\"><tr><td>";
            tooltip.append(QString("<html><b>%1</b>").arg(info.at(2)));
            tooltip.append("</td></tr></table>");
            if(!info.at(1).startsWith("http")) {
                tooltip.append(tr("<br />Current Time: %1/%2").arg(info.at(9))
                               .arg(info.at(6)));
            }
            tooltip.append(QString("<br /><img src='%1' width='300' />")
                           .arg(coverPath()));
        }
        tooltip.append("</html>");
    }
    else
        tooltip = tr("Player is not running, make a doubleclick.");
    trayIcon->setToolTip(tooltip);
}

void TrayIcon::showLyricsWindow() {
    QPointer<LyricsWindow> lyricsWindow = new LyricsWindow(this, m_player);
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

void TrayIcon::setQuitBehaviour() {
    if(setQuitBehaviourAction->isChecked()) {
        connect(quitAction, SIGNAL(triggered()), m_player, SLOT(quit()));
        m_settings->setValue("player/quitmoc", true);
    }
    else {
        disconnect(quitAction, 0, m_player, 0);
        m_settings->setValue("player/quitmoc", false);
    }
}

void TrayIcon::setScrobbling() {
    if(setScrobblingAction->isChecked()) {
        m_settings->setValue("scrobbler/enabled", true);
        if(!m_settings->value("scrobbler/sessionkey").toBool())
            showConfigurationDialog();
    }
    else {
        m_settings->setValue("scrobbler/enabled", false);
    }
}

void TrayIcon::showConfigurationDialog() {
    ScrobblerSettings *settingsDialog = new ScrobblerSettings(m_settings);
    settingsDialog->show();
}
