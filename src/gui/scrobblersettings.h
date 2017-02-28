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

#ifndef SCROBBLERSETTINGS_H
#define SCROBBLERSETTINGS_H

#include "basedialog.h"

class QLineEdit;
class QLabel;
class ScrobblerAuth;

class ScrobblerSettings : public BaseDialog
{
    Q_OBJECT

    ScrobblerAuth* scrobblerAuth;
    bool success;
    QLineEdit* usernameLineEdit;
    QLineEdit* passwordLineEdit;
    QLabel* label;

public:
    explicit ScrobblerSettings(QWidget *parent = nullptr);
    ~ScrobblerSettings();

private slots:
    void auth();
    void authFail(const QString& errmsg);
    void authSuccess();

signals:
    void configured(bool);
};

#endif // SCROBBLERSETTINGS_H
