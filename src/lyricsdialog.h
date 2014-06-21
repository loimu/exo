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

#ifndef LYRICSDIALOG_H
#define LYRICSDIALOG_H
#if QT_VERSION >= 0x050000
    #include <QtWidgets>
#endif

#include <QWidget>

#include "ui_lyricsdialog.h"

class QAction;
class QLabel;
class QNetworkAccessManager;
class QNetworkReply;

class LyricsDialog : public QWidget
{
    Q_OBJECT

    Ui::LyricsDialog ui;
    QNetworkAccessManager *httpObject;
    QNetworkReply *replyObject;
    QString artistString, titleString;
    void search();
    QString format(QString);

public:
    LyricsDialog(QWidget *parent = 0);

private slots:
    void showText(QNetworkReply *reply);
    void on_artistLineEdit_returnPressed();
    void on_titleLineEdit_returnPressed();
    void on_updatePushButton_clicked();
    void on_prevButton_clicked();
    void on_nextButton_clicked();
};

#endif // LYRICSDIALOG_H
