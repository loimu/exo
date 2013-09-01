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

#ifndef SCROBBLERSETTINGS_H
#define SCROBBLERSETTINGS_H

#include <QWidget>

class QSettings;

namespace Ui {
class ScrobblerSettings;
}

class ScrobblerSettings : public QWidget
{
    Q_OBJECT

    Ui::ScrobblerSettings *ui;
    QSettings* m_settings;

public:
    explicit ScrobblerSettings(QSettings* settings);
    ~ScrobblerSettings();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
};

#endif // SCROBBLERSETTINGS_H
