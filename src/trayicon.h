/* ========================================================================
*    Copyright (C) 2013-2015 Blaze <blaze@open.by>
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
class AboutDialog;
class QSettings;

class TrayIcon : public QWidget
{
    Q_OBJECT

    void createActions();
    void createTrayIcon();
    bool eventFilter(QObject *, QEvent *);
    AboutDialog *about;
    QMenu *trayIconMenu;
    QMenu *settingsMenu;
    QAction *showAction;
    QAction *filesAction;
    QAction *lyricsAction;
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

public:
    explicit TrayIcon(QObject *parent = 0);
    ~TrayIcon();

signals:
    void playerOpenWindow();
    void playerTogglePause();
    void playerVolumeUp();
    void playerVolumeDown();
    void playerNext();
    void playerPrevious();
    void playerForward();
    void playerRewind();

private slots:
    void clicked(QSystemTrayIcon::ActivationReason);
    void updateToolTip(QString, QString, QString, QString);
    void showAboutDialog();
    void setQuitBehaviour(bool);
    void addFiles();
};

#endif // TRAYICON_H
