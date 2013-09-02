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

#include <QSettings>

#include "scrobblersettings.h"
#include "ui_scrobblersettings.h"

ScrobblerSettings::ScrobblerSettings(QSettings *settings)
    : ui(new Ui::ScrobblerSettings) {

    ui->setupUi(this);
    m_settings = settings;
}

ScrobblerSettings::~ScrobblerSettings()
{
    delete ui;
}

void ScrobblerSettings::on_buttonBox_accepted() {
    if(ui->Username && ui->Password) {
        m_settings->setValue("scrobbler/enabled", true);
        m_settings->setValue("scrobbler/login", ui->Username->text());
        m_settings->setValue("scrobbler/password", ui->Password->text());
        m_settings->setValue("scrobbler/configured", true);
    }
    this->close();
}

void ScrobblerSettings::on_buttonBox_rejected() {
    m_settings->setValue("scrobbler/enabled", false);
    m_settings->setValue("scrobbler/configured", true);
    this->close();
}
