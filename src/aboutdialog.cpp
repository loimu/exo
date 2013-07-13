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

#include "aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QWidget(parent)
{
    ui.setupUi(this);
    setWindowFlags(Qt::Dialog);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_QuitOnClose, false);
    addText();
}

void AboutDialog::addText() {
    QString text;

    text.append("<html><head/><body>");
    text.append("<p><b>eXo</b> \251 2013 Blaze<br />&lt;blaze@jabster.pl&gt;</p>");
    text.append(tr("<p>Qt %1 (built with Qt %2)<br />").arg(qVersion()).arg(QT_VERSION_STR));
    text.append(tr("Licensed under GPL v3 or later.</p>"));
    text.append(tr("<p><b>Links:</b><br />"));
    text.append(tr("News: <a href=\"http://latitude52.tk/tag/exo/\">http://latitude52.tk/tag/exo/</a><br />"));
    text.append(tr("Bugreports: <a href=\"https://bitbucket.org/blaze/exo/issues\">https://bitbucket.org/blaze/exo/issues</a></p>"));
    text.append("</body></html>");

    ui.label->setText(text);
}
