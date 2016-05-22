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

#ifndef TRAYICON_H
#define TRAYICON_H

#include "config.h"

#include <QWidget>
#include <QSystemTrayIcon>

class PlayerInterface;
class QSettings;
class BookmarkManager;

class TrayIcon : public QWidget
{
    Q_OBJECT

    PlayerInterface *player;
    QMenu *trayIconMenu;
    QMenu *settingsMenu;
    QMenu *bookmarksMenu;
    QAction *showAction;
    QAction *filesAction;
    QAction *lyricsAction;
    QAction *bookmarkCurrentAction;
    QAction *bookmarkManagerAction;
    QAction *playAction;
    QAction *pauseAction;
    QAction *prevAction;
    QAction *nextAction;
    QAction *stopAction;
    QAction *aboutAction;
    QAction *quitAction;
    QAction *setQuitBehaviourAction;
    QAction *setScrobblingAction;
    QSystemTrayIcon *trayIcon;
    BookmarkManager *bookmarkManager;
    void createActions();
    void createTrayIcon();
    bool eventFilter(QObject *, QEvent *);

public:
    explicit TrayIcon(QObject *parent = nullptr);

private slots:
    void clicked(QSystemTrayIcon::ActivationReason);
    void updateToolTip(QString, QString, QString, QString);
    void showAboutDialog();
    void setQuitBehaviour(bool);
    void addFiles();
    void refreshBookmarks();
#ifdef BUILD_LASTFM
    void enableScrobbler(bool);
#endif // BUILD_LASTFM

public slots:
    void showLyricsWindow();
};

#endif // TRAYICON_H
