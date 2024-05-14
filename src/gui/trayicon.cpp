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

#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QActionGroup>
#include <QSystemTrayIcon>
#include <QWheelEvent>
#include <QSettings>
#include <QFileDialog>
#include <QCoreApplication>
#include <QProcess>
#include <QRegularExpression>
#include <QVector>

// core
#include "playerinterface.h"
#include "sysutils.h"
// gui
#include "aboutdialog.h"
#include "bookmarkmanager.h"
#include "lyricsdialog.h"
#include "trayicon.h"

#ifdef BUILD_LASTFM
  #include "scrobblersettings.h"
  #include "scrobbler.h"
#endif // BUILD_LASTFM

#define QSL QStringLiteral


const QVector<Provider> TrayIcon::providers = {
    {
        QSL("chartlyrics.com"),
        QSL("http://www.chartlyrics.com/search.aspx?q=%1+%2"),
        QSL("http://www.chartlyrics.com%1"),
        QSL("<td>[^\\w]+<a href=\"([^\"]*)"),
        QSL("<p>(.*)</p>"),
        {{QSL("\r\n"), QString()}},
        {QSL("<img class=\"alignright\".*?/>")}
    },
    {
        QSL("elyrics.net"),
        QSL("https://www.elyrics.net/read/%3/%1-lyrics/%2-lyrics.html"),
        QString(), QString(),
        QSL("<div id='inlyr'>(.*)</div><br>"),
        {{QSL("<br>"), QString()}},
        {QSL("<div.+div>")}
    },
    {
        QSL("songlyrics.com"),
        QSL("https://www.songlyrics.com/%1/%2-lyrics/"),
        QString(), QString(),
        QSL("<p id=\"songLyricsDiv\".*?\">(.*?)</p>"),
        {{QSL("\r\n"), QString()}},
        {}
    },
    {
        QSL("lyrics.ovh"),
        QSL("https://api.lyrics.ovh/v1/%1/%2"),
        QString(), QString(),
        QSL("{\"lyrics\":\"(.*)\"}"),
        {{QSL("\\n"), QSL("\n")}, {QSL("\\r"), QString()}},
        {}
    }
};

TrayIcon* TrayIcon::object = nullptr;

TrayIcon::TrayIcon(QWidget* parent) : QWidget(parent),
    player(PLAYER)
{
    setAttribute(Qt::WA_DontShowOnScreen);
    object = this;
    BookmarkManager::migrateBookmarks();
    createActions();
    createTrayIcon();
    connect(player, &PlayerInterface::newStatus, this, &TrayIcon::updateStatus);
    connect(player, &PlayerInterface::newTrack, this, &TrayIcon::updateTrack);
    isPlatformTrayBroken = qgetenv("XDG_CURRENT_DESKTOP") == QByteArray("KDE");
}

void TrayIcon::createActions() {
    showAction = new QAction(tr("Player"), this);
    connect(showAction, &QAction::triggered,
            player, &PlayerInterface::showPlayer);
    filesAction = new QAction(tr("A&dd ..."), this);
    connect(filesAction, &QAction::triggered, this, &TrayIcon::addFiles);
    lyricsAction = new QAction(tr("Show Lyrics"), this);
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
    connect(bookmarkCurrentAction, &QAction::triggered, this, &TrayIcon::bookmarkCurrent);
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
    auto trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(showAction);
    trayIconMenu->addAction(filesAction);
    // Lyrics submenu
    auto lyricsMenu = new QMenu(this);
    lyricsMenu->setTitle(tr("&Lyrics"));
    trayIconMenu->addAction(lyricsMenu->menuAction());
    lyricsMenu->addAction(lyricsAction);
    lyricsMenu->addSeparator();
    lyricsMenuGroup = new QActionGroup(this);
    int counter = 0;
    for(const Provider& provider : providers) {
        auto lyricsAction = new QAction(provider.name, this);
        lyricsAction->setCheckable(true);
        lyricsAction->setActionGroup(lyricsMenuGroup);
        lyricsAction->setData(counter);
        lyricsMenu->addAction(lyricsAction);
        counter++;
    }
    lyricsMenuGroup->actions().at(0)->setChecked(true);
    // end of Lyrics submenu
    const QVector<QString> editors = SysUtils::findFullPaths(
                QVector<QString> {
                    QStringLiteral("picard"),
                    QStringLiteral("kid3"),
                    QStringLiteral("easytag"),
                    QStringLiteral("puddletag") }); // detects tag editors
    if(!editors.isEmpty()) {
        auto tagEditorsMenu = new QMenu(trayIconMenu);
        tagEditorsMenu->setTitle(tr("&Edit with"));
        trayIconMenu->addAction(tagEditorsMenu->menuAction());
        connect(tagEditorsMenu, &QMenu::triggered, this, [] (QAction* action) {
            if(!PLAYER->getTrack().isStream)
                QProcess::startDetached(action->data().toString(),
                                        QStringList{PLAYER->getTrack().file});
        });
        for(const QString& entry : editors) {
            const QString app = entry.split(QChar::fromLatin1('/')).last();
            auto newEditor = new QAction(app.at(0).toUpper() + app.mid(1),this);
            newEditor->setIcon(QIcon::fromTheme(app));
            newEditor->setData(entry);
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
    createBookmarks();
    // end of Bookmarks submenu
    // Settings submenu
    auto settingsMenu = new QMenu(trayIconMenu);
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
    trayIcon->setIcon(QIcon::fromTheme("exo", icon));
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
    if(event->type() == QEvent::ToolTip) {
        if(playerState == PState::Play
                && !player->getTrack().isStream)
            updateTrack(coverArt, true);
        return true;
    }
    if(event->type() == QEvent::Wheel) {
        QWheelEvent* e = static_cast<QWheelEvent*>(event);
        player->changeVolume(e->angleDelta().y() / 100);
        return true;
    }
    return false;
}

void TrayIcon::updateStatus(PState state) {
    playerState = state;
    if(state < PState::Play)
        trayIcon->setToolTip(state < PState::Stop ? tr("Player isn't running")
                                                  : tr("Stopped") );
}

void TrayIcon::updateTrack(const QString& cover, bool toolTipEvent) {
    QString tooltip, time;
    const PTrack& track = player->getTrack();
    if(toolTipEvent) {
        time = QString(QStringLiteral("%1:%2/%3"))
                .arg(track.currSec / 60)
                .arg(track.currSec % 60, 2, 10, QChar::fromLatin1('0'))
                .arg(track.totalTime);
    } else {
        time = track.totalTime;
        coverArt = cover;
    }
    if(track.isStream) {
        tooltip = track.caption;
    } else {
        /* try to guess a year from the file path
         * only years starting with 19, 20  are considered to be valid
         *  in order to exclude false positives as much as possible */
        static const QRegularExpression re(QStringLiteral("((19|20){1}\\d{2})"));
        QRegularExpressionMatch match = re.match(track.file);
        /* only tooltips with fixed size have acceptable look in some DEs
         * therefore we are using a fixed-size table here */
        tooltip = isPlatformTrayBroken
                ? QStringLiteral("%1 %2 (%3)").arg(
                      track.caption, match.captured(1), time)
                : QStringLiteral("<table width=\"320\"><tr><td><b>%1 %2 (%3)"
                                   "</b></td></tr></table><br /><img src=\"%4\""
                                   " width=\"320\" height=\"320\" />").arg(
                      track.caption, match.captured(1), time,
                      cover.isEmpty() ? QStringLiteral(":/images/nocover.png")
                                      : cover);
    }
    trayIcon->setToolTip(tooltip);
}

void TrayIcon::showLyricsWindow() {
    int index = lyricsMenuGroup->checkedAction()->data().toInt();
    auto lyricsDialog = new LyricsDialog(providers.at(index), this);
    lyricsDialog->show();
}

void TrayIcon::showAboutDialog() {
    aboutAction->setEnabled(false);
    auto about = new AboutDialog(this);
    about->show();
    connect(about, &AboutDialog::destroyed,
            this, [this] { aboutAction->setEnabled(true); });
}

void TrayIcon::setQuitBehaviour(bool checked) {
    QSettings settings;
    settings.setValue(QStringLiteral("player/quit"), checked);
}

void TrayIcon::addFiles() {
    const QStringList files = QFileDialog::getOpenFileNames(
                this,
                QStringLiteral("Add files to playlist"),
                QString(),
                QStringLiteral(
                    "Media (*.aac *.flac *.m4a *.mp3 *.ogg *.wma);;"
                    "Playlists (*.m3u *.pls *.xspf);;"
                    "Media and Playlists "
                    "(*.aac *.flac *.m4a *.mp3 *.ogg *.wma *.m3u *.pls *.xspf);;"
                    "All files (*)"));
    player->appendFile(files);
}

void TrayIcon::bookmarkCurrent() {
    const QString& url = PLAYER->getTrack().file;
    if(!url.isEmpty() && PLAYER->getTrack().isStream) {
        QSettings settings;
        QList<QVariant> list = settings.value("bookmarkmanager/bookmarks2").toList();
        const QString name = url.split(QChar::fromLatin1('/'), Qt::SkipEmptyParts).last();
        list.append(QStringList{name, url});
        settings.setValue("bookmarkmanager/bookmarks2", list);
        refreshBookmarks(list);
    } else {
        qInfo("invalid bookmark");
    }
}

void TrayIcon::showManager() {
    bookmarkManagerAction->setEnabled(false);
    auto bm = new BookmarkManager(this);
    bm->show();
    connect(bm, &BookmarkManager::destroyed,
            this, [this] { bookmarkManagerAction->setEnabled(true); });
}

void TrayIcon::createBookmarks() {
    bookmarksMenu->addAction(bookmarkCurrentAction);
    bookmarksMenu->addAction(bookmarkManagerAction);
    bookmarksMenu->addSeparator();
    QSettings settings;
    refreshBookmarks(settings.value("bookmarkmanager/bookmarks2").toList());
}

void TrayIcon::refreshBookmarks(const QList<QVariant>& bookmarks) {
    const auto actions = bookmarksMenu->actions();

    for(QAction* action : actions) {
        if(!(action == bookmarkCurrentAction || action == bookmarkManagerAction
              || action->isSeparator())) {
            action->deleteLater();
        }
    }
    for(const auto& entry : bookmarks) {
        const auto& bookmark = entry.toStringList();
        if(bookmark.count() < 2) continue;
        const QString& key = bookmark.at(0);
        auto action = new QAction(key, bookmarksMenu);
        action->setData(bookmark.at(1));
        bookmarksMenu->addAction(action);
        connect(action, &QAction::triggered, this, [action] () {
            PLAYER->openUri(action->data().toString());
        });
    }
}

#ifdef BUILD_LASTFM
void TrayIcon::enableScrobbler(bool checked) {
    QSettings settings;
    if(settings.value(QStringLiteral("scrobbler/sessionkey")).toBool()) {
        settings.setValue(QStringLiteral("scrobbler/enabled"), checked);
        loadScrobbler(checked);
    } else {
        setScrobblingAction->setChecked(false);
        auto settingsDialog = new ScrobblerSettings(this);
        settingsDialog->show();
        connect(settingsDialog, &ScrobblerSettings::configured, this, [this] {
            setScrobblingAction->setChecked(true);
            loadScrobbler(true);
        });
    }
}

void TrayIcon::loadScrobbler(bool checked) {
    Scrobbler* scrobbler = Scrobbler::self();
    if(!scrobbler && checked)
        new Scrobbler(this);
    else if(scrobbler && !checked)
        scrobbler->deleteLater();
}
#endif // BUILD_LASTFM
