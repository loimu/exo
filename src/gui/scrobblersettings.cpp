/* ========================================================================
*    Copyright (C) 2013-2016 Blaze <blaze@vivaldi.net>
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

#include "core/exo.h"
#include "lastfm/scrobblerauth.h"
#include "scrobblersettings.h"
#include "ui_scrobblersettings.h"

ScrobblerSettings::ScrobblerSettings(QWidget *parent) : BaseDialog(parent),
    success(false),
    ui(new Ui::ScrobblerSettings)
{
    ui->setupUi(this);
    scrobblerAuth = new ScrobblerAuth(this);
    connect(scrobblerAuth, SIGNAL(failed(QString)), SLOT(authFail(QString)));
    connect(scrobblerAuth, SIGNAL(configured()), SLOT(authSuccess()));
    connect(ui->usernameLineEdit, SIGNAL(returnPressed()),
            SLOT(on_buttonBox_accepted()));
    connect(ui->passwordLineEdit, SIGNAL(returnPressed()),
            SLOT(on_buttonBox_accepted()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(close()));
}

ScrobblerSettings::~ScrobblerSettings() {
    emit configured(success);
}

void ScrobblerSettings::on_buttonBox_accepted() {
    scrobblerAuth->auth(ui->usernameLineEdit->text(),
                        ui->passwordLineEdit->text());
}

void ScrobblerSettings::on_usernameLineEdit_textChanged() {
    ui->label->setText("");
}

void ScrobblerSettings::on_passwordLineEdit_textChanged() {
    ui->label->setText("");
}

void ScrobblerSettings::authFail(const QString& errmsg) {
    ui->label->setText(errmsg);
}

void ScrobblerSettings::authSuccess() {
    success = true;
    this->close();
}
