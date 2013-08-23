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

#ifndef TRAYICON_H
#define TRAYICON_H
#if QT_VERSION >= 0x050000
    #include <QtWidgets>
#endif

#include <QWidget>
#include <QPointer>
#include <QSystemTrayIcon>

class PlayerInterface;
class AboutDialog;

class TrayIcon : public QWidget
{
    Q_OBJECT
public:
    explicit TrayIcon(PlayerInterface *player = 0);

signals:

private slots:
    void clicked(QSystemTrayIcon::ActivationReason);
    void updateToolTip();
    void showLyricsWindow();
    void showAboutDialog();

public slots:

private:
    void createActions();
    void createTrayIcon();
    QString coverPath();

    PlayerInterface *m_player;
    QPointer<AboutDialog> m_about;

    QMenu *trayIconMenu;

    QAction *lyricsAction;
    QAction *playAction;
    QAction *pauseAction;
    QAction *prevAction;
    QAction *nextAction;
    QAction *stopAction;
    QAction *aboutAction;
    QAction *quitAction;

    QSystemTrayIcon *trayIcon;

};

#endif // TRAYICON_H
