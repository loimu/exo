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

#ifndef SCROBBLERSETTINGS_H
#define SCROBBLERSETTINGS_H
#if QT_VERSION >= 0x050000
    #include <QtWidgets>
#endif

#include <QWidget>

class QSettings;

namespace lastfm {
class XmlQuery;
}

namespace Ui {
class ScrobblerSettings;
}

class ScrobblerSettings : public QWidget
{
    Q_OBJECT

    void auth(const QString& username, const QString& password);
    lastfm::XmlQuery EmptyXmlQuery();
    bool ParseQuery(const QByteArray& data, lastfm::XmlQuery* query,
                    bool* connectionProblems = NULL);
    Ui::ScrobblerSettings *ui;

public:
    explicit ScrobblerSettings(QObject *parent = 0);
    ~ScrobblerSettings();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_usernameLineEdit_returnPressed();
    void on_passwordLineEdit_returnPressed();
    void on_usernameLineEdit_textChanged();
    void on_passwordLineEdit_textChanged();
    void authReplyFinished();

signals:
    void configured();
};

#endif // SCROBBLERSETTINGS_H
