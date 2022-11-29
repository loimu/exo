/* ========================================================================
*    Copyright (C) 2013-2022 Blaze <blaze@vivaldi.net>
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

#include "basedialog.h"

class QLabel;
class QTextBrowser;
class QLineEdit;
class QNetworkAccessManager;
class QNetworkReply;
struct Provider;

class LyricsDialog : public BaseDialog
{
    Q_OBJECT

public:
    explicit LyricsDialog(QWidget* parent = nullptr);

private:
    static const QVector<Provider> providers;
    int index = 0;
    QLabel* label;
    QTextBrowser* lyricsBrowser;
    QLineEdit* artistLineEdit;
    QLineEdit* titleLineEdit;
    QNetworkAccessManager* httpObject;
    QNetworkReply* replyObject;
    QString artistString, titleString;
    QString replace(QString string);
    void showText(QNetworkReply* reply);
    void update();
    void search();
};

#endif // LYRICSDIALOG_H
