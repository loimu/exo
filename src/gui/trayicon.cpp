/* ========================================================================
*    Copyright (C) 2013-2016 Blaze <blaze@vivaldi.net>
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
#include <QSettings>
#include <QFileDialog>
#include <QPointer>
#include <QProcess>

#include "core/exo.h"
#include "core/playerinterface.h"
#include "lyricsdialog.h"
#include "aboutdialog.h"
#include "scrobblersettings.h"
#include "bookmarks/bookmarkmanager.h"
#include "bookmarks/bookmark.h"
#include "editor.h"
#include "trayicon.h"

TrayIcon* TrayIcon::object = nullptr;

TrayIcon::TrayIcon(QObject *parent) :
    bookmarkManager(new BookmarkManager(this)),
    player(PlayerInterface::self())
{
    object = this;
    createActions();
    createTrayIcon();
    connect(player, SIGNAL(updateStatus(QString, QString, QString, QString)),
            SLOT(updateToolTip(QString, QString, QString, QString)));
    connect(bookmarkManager, SIGNAL(refreshBookmarks()), SLOT(refreshBookmarks()));
}

TrayIcon* TrayIcon::self() {
    return object;
}

void TrayIcon::createActions() {
    showAction = new QAction(tr("Player"), this);
    connect(showAction, SIGNAL(triggered()), player, SLOT(showPlayer()));
    filesAction = new QAction(tr("A&dd ..."), this);
    connect(filesAction, SIGNAL(triggered()), SLOT(addFiles()));
    lyricsAction = new QAction(tr("&Lyrics"), this);
    connect(lyricsAction, SIGNAL(triggered()), SLOT(showLyricsWindow()));
    playAction = new QAction(tr("&Play"), this);
    connect(playAction, SIGNAL(triggered()), player, SLOT(play()));
    QIcon playIcon(":/images/play.png");
    playAction->setIcon(playIcon);
    pauseAction = new QAction(tr("P&ause"), this);
    connect(pauseAction, SIGNAL(triggered()), player, SLOT(playPause()));
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
    connect(aboutAction, SIGNAL(triggered()), SLOT(showAboutDialog()));
    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    QIcon quitIcon(":/images/close.png");
    quitAction->setIcon(quitIcon);
    bookmarkCurrentAction = new QAction(tr("Bookmark Current"), this);
    connect(bookmarkCurrentAction, SIGNAL(triggered()), bookmarkManager, SLOT(addCurrent()));
    bookmarkManagerAction = new QAction(tr("Bookmark Manager"), this);
    connect(bookmarkManagerAction, SIGNAL(triggered()), bookmarkManager, SLOT(manager()));
    setQuitBehaviourAction = new QAction(tr("&Close player on exit"), this);
    setQuitBehaviourAction->setCheckable(true);
    QSettings settings;
    setQuitBehaviourAction->setChecked(settings.value("player/quit").toBool());
    connect(setQuitBehaviourAction, SIGNAL(triggered(bool)),
            SLOT(setQuitBehaviour(bool)));

#ifdef BUILD_LASTFM
    setScrobblingAction = new QAction(tr("&Enable scrobbling"), this);
    setScrobblingAction->setCheckable(true);
    setScrobblingAction->setChecked(settings.value("scrobbler/enabled").toBool());
    connect(setScrobblingAction, SIGNAL(triggered(bool)), SLOT(enableScrobbler(bool)));
#endif // BUILD_LASTFM
}

void TrayIcon::createTrayIcon() {
    // creating menu and adding actions
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(showAction);
    trayIconMenu->addAction(filesAction);
    trayIconMenu->addAction(lyricsAction);
    // detect tag editors
    QProcess proc;
    proc.start("which", QStringList() << "picard" << "kid3" << "easytag" << "puddletag");
    proc.waitForFinished(-1);
    QStringList editors = QString::fromUtf8(proc.readAllStandardOutput())
            .split("\n", QString::SkipEmptyParts);
    if(editors.length() > 0) {
        QMenu* tagEditorsMenu = new QMenu(this);
        tagEditorsMenu->setTitle(tr("Edit with"));
        trayIconMenu->addAction(tagEditorsMenu->menuAction());
        for(QString entry : editors) {
            Editor* newEditor = new Editor(entry, this);
            tagEditorsMenu->addAction(newEditor);
        }
    }
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(playAction);
    trayIconMenu->addAction(pauseAction);
    trayIconMenu->addAction(prevAction);
    trayIconMenu->addAction(nextAction);
    trayIconMenu->addAction(stopAction);
    trayIconMenu->addSeparator();
    // Bookmarks submenu
    bookmarksMenu = new QMenu(trayIconMenu);
    bookmarksMenu->setTitle(tr("Links"));
    trayIconMenu->addAction(bookmarksMenu->menuAction());
    refreshBookmarks();
    // end of Bookmarks submenu
    // Settings submenu
    settingsMenu = new QMenu(trayIconMenu);
    settingsMenu->setTitle(tr("Se&ttings"));
    trayIconMenu->addAction(settingsMenu->menuAction());
    settingsMenu->addAction(setQuitBehaviourAction);

#ifdef BUILD_LASTFM
    settingsMenu->addAction(setScrobblingAction);
#endif // BUILD_LASTFM

    // end of Settings submenu
    trayIconMenu->addAction(aboutAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
    // tray icon setup
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    QIcon icon(":/images/32.png");
    trayIcon->setIcon(icon);
    // event filter needed for corresponding method in this class
    trayIcon->installEventFilter(this);
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                      SLOT(clicked(QSystemTrayIcon::ActivationReason)));
    trayIcon->show();
}

void TrayIcon::clicked(QSystemTrayIcon::ActivationReason reason) {
    switch(reason) {
    case QSystemTrayIcon::DoubleClick:
        player->showPlayer();
        break;
    case QSystemTrayIcon::Trigger:
        break;
    case QSystemTrayIcon::MiddleClick:
        player->playPause();
        break;
    default:
        break;
    }
}

bool TrayIcon::eventFilter(QObject* object, QEvent* event) {
    if(QObject::eventFilter(object, event))
        return true;
    if(object != trayIcon)
        return false;
    if(event->type() == QEvent::Wheel) {
        QWheelEvent* e = static_cast<QWheelEvent*>(event);
        player->changeVolume(e->delta()/100);
        return true;
    }
    return false;
}

void TrayIcon::updateToolTip(QString message, QString currentTime,
                             QString totalTime, QString cover) {
    // it seems that tooltip with fixed size looks better
    QString tooltip = "<table width=\"300\"><tr><td><b>" + message +
            "</b></td></tr></table>";
    if(!cover.isEmpty()) {
        tooltip.append(QString("<br />Current time: %1/%2<br />"
                               "<img src=\"%3\" width=\"300\" height=\"300\" />")
                       .arg(currentTime).arg(totalTime).arg(cover));
    }
    trayIcon->setToolTip(tooltip);
}

void TrayIcon::showLyricsWindow() {
    QPointer<LyricsDialog> lyricsDialog = new LyricsDialog();
    lyricsDialog->show();
}

void TrayIcon::showAboutDialog() {
    aboutAction->setDisabled(true);
    AboutDialog* about = new AboutDialog(this);
    about->show();
    connect(about, SIGNAL(destroyed(bool)), aboutAction, SLOT(setEnabled(bool)));
}

void TrayIcon::setQuitBehaviour(bool checked) {
    QSettings settings;
    settings.setValue("player/quit", checked);
}

void TrayIcon::addFiles() {
    QStringList files = QFileDialog::getOpenFileNames(this, "Add files to"
                                                            " playlist", "",
                                   "Media (*.pls *.m3u *.ogg *.mp3 *.flac)");
    player->appendFile(files);
}

void TrayIcon::refreshBookmarks() {
    bookmarksMenu->clear();
    bookmarksMenu->addAction(bookmarkCurrentAction);
    if(bookmarkManager->bookmarks()->length() < 1)
        return;
    bookmarksMenu->addAction(bookmarkManagerAction);
    bookmarksMenu->addSeparator();
    for(BookmarkEntry entry : *bookmarkManager->bookmarks()) {
        Bookmark *bookmark = new Bookmark(entry.name, this);
        bookmark->uri = entry.uri;
        bookmarksMenu->addAction(bookmark);
    }
}

#ifdef BUILD_LASTFM
void TrayIcon::enableScrobbler(bool checked) {
    QSettings settings;
    if(settings.value("scrobbler/sessionkey").toBool()) {
        settings.setValue("scrobbler/enabled", checked);
        Exo::self()->loadScrobbler(checked);
    } else
        if(checked) {
            ScrobblerSettings *settingsDialog = new ScrobblerSettings(this);
            settingsDialog->show();
            connect(settingsDialog, SIGNAL(configured(bool)),
                    setScrobblingAction, SLOT(setChecked(bool)));
        }
}
#endif // BUILD_LASTFM
