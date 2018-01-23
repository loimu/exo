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

#ifndef TRAYICON_H
#define TRAYICON_H

#include "config.h"

#include <QWidget>
#include <QSystemTrayIcon>
#include <QVector>

class PlayerInterface;
class BookmarkEntry;

class TrayIcon : public QWidget
{
    static TrayIcon* object;
    PlayerInterface* player;
    QMenu*bookmarksMenu;
    QAction* showAction;
    QAction* filesAction;
    QAction* lyricsAction;
    QAction* bookmarkCurrentAction;
    QAction* bookmarkManagerAction;
    QAction* playAction;
    QAction* pauseAction;
    QAction* prevAction;
    QAction* nextAction;
    QAction* stopAction;
    QAction* aboutAction;
    QAction* quitAction;
    QAction* setQuitBehaviourAction;
    QAction* setScrobblingAction;
    QSystemTrayIcon* trayIcon;
    void createActions();
    void createTrayIcon();
    bool eventFilter(QObject*, QEvent*);
    void clicked(QSystemTrayIcon::ActivationReason);
    void updateToolTip(const QString& message, const QString& cover);
    void showAboutDialog();
    void setQuitBehaviour(bool);
    void addFiles();
    void showManager();
#ifdef BUILD_LASTFM
    void enableScrobbler(bool);
    void loadScrobbler(bool);
#endif // BUILD_LASTFM

public:
    explicit TrayIcon(QWidget* parent = nullptr);
    static TrayIcon* self() { return object; }
    void refreshBookmarks(const QVector<BookmarkEntry>& list);
    void showLyricsWindow();
    void addCurrent();
};

#endif // TRAYICON_H
