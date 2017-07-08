/* ========================================================================
*    Copyright (C) 2013-2017 Blaze <blaze@vivaldi.net>
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
#include <QCoreApplication>

#include "bookmarks/bookmark.h"
#include "bookmarks/bookmarkmanager.h"
#include "core/playerinterface.h"
#include "core/process.h"
#include "gui/lyricsdialog.h"
#include "gui/aboutdialog.h"
#include "gui/tageditor.h"
#ifdef BUILD_LASTFM
  #include "gui/scrobblersettings.h"
  #include "lastfm/scrobbler.h"
#endif // BUILD_LASTFM
#include "trayicon.h"

TrayIcon* TrayIcon::object = nullptr;

TrayIcon::TrayIcon(QWidget* parent) : QWidget(parent),
    player(PlayerInterface::self())
{
    setAttribute(Qt::WA_DontShowOnScreen);
    object = this;
    createActions();
    createTrayIcon();
    connect(player, &PlayerInterface::updateStatus,
            this, &TrayIcon::updateToolTip);
}

void TrayIcon::createActions() {
    showAction = new QAction(tr("Player"), this);
    connect(showAction, &QAction::triggered,
            player, &PlayerInterface::showPlayer);
    filesAction = new QAction(tr("A&dd ..."), this);
    connect(filesAction, &QAction::triggered, this, &TrayIcon::addFiles);
    lyricsAction = new QAction(tr("&Lyrics"), this);
    connect(lyricsAction, &QAction::triggered,
            this, &TrayIcon::showLyricsWindow);
    playAction = new QAction(tr("&Play"), this);
    connect(playAction, &QAction::triggered, player, &PlayerInterface::play);
    playAction->setIcon(QIcon(QStringLiteral(":/images/play.png")));
    pauseAction = new QAction(tr("P&ause"), this);
    connect(pauseAction, &QAction::triggered,
            player, &PlayerInterface::playPause);
    pauseAction->setIcon(QIcon(QStringLiteral(":/images/pause.png")));
    prevAction = new QAction(tr("P&rev"), this);
    connect(prevAction, &QAction::triggered, player, &PlayerInterface::prev);
    prevAction->setIcon(QIcon(QStringLiteral(":/images/prev.png")));
    nextAction = new QAction(tr("&Next"), this);
    connect(nextAction, &QAction::triggered, player, &PlayerInterface::next);
    nextAction->setIcon(QIcon(QStringLiteral(":/images/next.png")));
    stopAction = new QAction(tr("&Stop"), this);
    connect(stopAction, &QAction::triggered, player, &PlayerInterface::stop);
    stopAction->setIcon(QIcon(QStringLiteral(":/images/stop.png")));
    aboutAction = new QAction(tr("A&bout"), this);
    connect(aboutAction, &QAction::triggered, this, &TrayIcon::showAboutDialog);
    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
    quitAction->setIcon(QIcon(QStringLiteral(":/images/close.png")));
    bookmarkCurrentAction = new QAction(tr("Bookmark &Current"), this);
    connect(bookmarkCurrentAction, &QAction::triggered,
            this, &TrayIcon::addCurrent);
    bookmarkCurrentAction->setIcon(
                QIcon::fromTheme(QStringLiteral("bookmark-new-list")));
    bookmarkManagerAction = new QAction(tr("Bookmark &Manager"), this);
    connect(bookmarkManagerAction, &QAction::triggered,
            this, &TrayIcon::showManager);
    bookmarkManagerAction->setIcon(
                QIcon::fromTheme(QStringLiteral("bookmarks-organize")));
    setQuitBehaviourAction = new QAction(tr("&Close player on exit"), this);
    setQuitBehaviourAction->setCheckable(true);
    QSettings settings;
    setQuitBehaviourAction->setChecked(
                settings.value(QStringLiteral("player/quit")).toBool());
    connect(setQuitBehaviourAction, &QAction::triggered,
            this, &TrayIcon::setQuitBehaviour);

#ifdef BUILD_LASTFM
    setScrobblingAction = new QAction(tr("&Enable scrobbling"), this);
    setScrobblingAction->setCheckable(true);
    setScrobblingAction->setChecked(
                settings.value(QStringLiteral("scrobbler/enabled")).toBool());
    connect(setScrobblingAction, &QAction::triggered,
            this, &TrayIcon::enableScrobbler);
#endif // BUILD_LASTFM
}

void TrayIcon::createTrayIcon() {
    // creating menu and adding actions
    QMenu* trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(showAction);
    trayIconMenu->addAction(filesAction);
    trayIconMenu->addAction(lyricsAction);
    QStringList editors = Process::detect(
                QStringList{
                    QStringLiteral("picard"),
                    QStringLiteral("kid3"),
                    QStringLiteral("easytag"),
                    QStringLiteral("puddletag")}); // detects tag editors
    if(!editors.isEmpty()) {
        QMenu* tagEditorsMenu = new QMenu(trayIconMenu);
        tagEditorsMenu->setTitle(tr("&Edit with"));
        trayIconMenu->addAction(tagEditorsMenu->menuAction());
        for(QString entry : editors) {
            TagEditor* newEditor = new TagEditor(entry, this);
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
    bookmarksMenu->setTitle(tr("Lin&ks"));
    trayIconMenu->addAction(bookmarksMenu->menuAction());
    refreshBookmarks(BookmarkManager::getList());
    // end of Bookmarks submenu
    // Settings submenu
    QMenu* settingsMenu = new QMenu(trayIconMenu);
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
    QIcon icon(QStringLiteral(":/images/32.png"));
    trayIcon->setIcon(icon);
    // event filter needed for corresponding method in this class
    trayIcon->installEventFilter(this);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &TrayIcon::clicked);
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

void TrayIcon::updateToolTip(const QString& message, const QString& cover) {
    /* only fixed sized tooltip has an acceptable look in some DEs */
    QString tooltip = QLatin1String("<table width=\"320\"><tr><td><b>")
            + message + QLatin1String("</b></td></tr></table>");
    if(!cover.isEmpty())
        tooltip.append(
                    QString(QLatin1String("<br /><img src=\"%1\" width=\"320\" "
                                          "height=\"320\" />")).arg(cover));
    trayIcon->setToolTip(tooltip);
}

void TrayIcon::showLyricsWindow() {
    LyricsDialog* lyricsDialog = new LyricsDialog(this);
    lyricsDialog->show();
}

void TrayIcon::showAboutDialog() {
    aboutAction->setEnabled(false);
    AboutDialog* about = new AboutDialog(this);
    about->show();
    connect(about, &AboutDialog::destroyed, aboutAction, &QAction::setEnabled);
}

void TrayIcon::setQuitBehaviour(bool checked) {
    QSettings settings;
    settings.setValue(QStringLiteral("player/quit"), checked);
}

void TrayIcon::addFiles() {
    QStringList files = QFileDialog::getOpenFileNames(
                this,
                QStringLiteral("Add files to playlist"),
                QString(),
                QStringLiteral("Media (*.pls *.m3u *.ogg *.mp3 *.flac)"));
    player->appendFile(files);
}

void TrayIcon::addCurrent() {
    BookmarkList bl = BookmarkManager::addCurrent();
    refreshBookmarks(bl);
}

void TrayIcon::showManager() {
    bookmarkManagerAction->setEnabled(false);
    BookmarkManager* bm = new BookmarkManager(this);
    bm->show();
    connect(bm, &BookmarkManager::destroyed,
            bookmarkManagerAction, &QAction::setEnabled);
}

void TrayIcon::refreshBookmarks(const BookmarkList& list) {
    bookmarksMenu->clear();
    bookmarksMenu->addAction(bookmarkCurrentAction);
    if(list.isEmpty())
        return;
    bookmarksMenu->addAction(bookmarkManagerAction);
    bookmarksMenu->addSeparator();
    for(BookmarkEntry entry : list) {
        Bookmark *bookmark = new Bookmark(entry.name, this);
        bookmark->uri = entry.uri;
        bookmarksMenu->addAction(bookmark);
    }
}

#ifdef BUILD_LASTFM
void TrayIcon::enableScrobbler(bool checked) {
    QSettings settings;
    if(settings.value(QStringLiteral("scrobbler/sessionkey")).toBool()) {
        settings.setValue(QStringLiteral("scrobbler/enabled"), checked);
        loadScrobbler(checked);
    } else
        if(checked) {
            ScrobblerSettings *settingsDialog = new ScrobblerSettings(this);
            settingsDialog->show();
            connect(settingsDialog, &ScrobblerSettings::configured,
                    setScrobblingAction, &QAction::setChecked);
            connect(settingsDialog, &ScrobblerSettings::configured,
                    this, &TrayIcon::loadScrobbler);
        }
}

void TrayIcon::loadScrobbler(bool checked) {
    QPointer<Scrobbler> scrobbler = Scrobbler::self();
    if(!scrobbler && checked)
        scrobbler = new Scrobbler(this);
    else if(scrobbler && !checked)
        scrobbler->deleteLater();
}
#endif // BUILD_LASTFM
