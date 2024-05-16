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

#ifndef LYRICSDIALOG_H
#define LYRICSDIALOG_H

#include "basedialog.h"
#include <QRegularExpression>


class QLabel;
class QTextBrowser;
class QLineEdit;
class QNetworkAccessManager;
class QNetworkReply;

struct Provider {
    const QString name;
    const QString searchUrl;
    const QString urlTemplate;
    const QString urlRegExp;
    const QString dataRegExp;
    const QVector<QPair<QString, QString>> replaceList;
    const QVector<QString> excludeList;
};

class LyricsDialog : public BaseDialog
{
    Q_OBJECT

public:
    explicit LyricsDialog(int providerNum = -1, QWidget* parent = nullptr);

private:
    bool cycleThroughProviders = false;
    QLabel* label;
    QTextBrowser* lyricsBrowser;
    QLineEdit* artistLineEdit;
    QLineEdit* titleLineEdit;
    QNetworkAccessManager* httpObject;
    QNetworkReply* replyObject;
    int providerNum;
    QString artistString, titleString;
    const QRegularExpression rgData;
    QString replace(QString string);
    QString escapeRegexInput(QString string);
    void showText(QNetworkReply* reply);
    void checkNextProvider();
    void update();
    void search();
};

#endif // LYRICSDIALOG_H
