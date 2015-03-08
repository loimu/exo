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

#include "aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) : QWidget(parent) {
    ui.setupUi(this);
    setWindowFlags(Qt::Dialog);
    setAttribute(Qt::WA_DeleteOnClose);
    addText();
}

void AboutDialog::addText() {
    QString text;
    text.append(QString("<p><b>eXo</b> v%1 &copy; 2013-2015 Blaze<br />&lt;blaze@"
                        "open.by&gt;</p><p>Qt %2 (built with Qt %3)<br />"
                        "Licensed under GPL v3 or later.</p><p><b>Links:</b>"
                        "<br />News: <a href=\"http://exo.loimu.tk/\">"
                        "http://exo.loimu.tk/</a><br />Project: "
                        "<a href=\"https://bitbucket.org/blaze/exo\">"
                        "https://bitbucket.org/blaze/exo</a><br />Bugs: "
                        "<a href=\"https://bitbucket.org/blaze/exo/issues\">"
                        "https://bitbucket.org/blaze/exo/issues</a></p>")
                .arg(qApp->applicationVersion())
                .arg(qVersion())
                .arg(QT_VERSION_STR));
    ui.label->setText(text);
}
