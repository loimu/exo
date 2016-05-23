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

#include <QProcess>

#include "core/playerinterface.h"
#include "editor.h"

Editor::Editor(const QString &text, QObject *parent) : QAction(text, parent)
{
    editorPath = text;
    QString app = text.split("/").last();
    this->setText(app.left(1).toUpper() + app.mid(1));
    connect(this, SIGNAL(triggered(bool)), this, SLOT(open()));
}

void Editor::open() {
    QString file = PlayerInterface::self()->trackObject()->file;
    if(file.startsWith("/")) {
        QProcess proc;
        proc.start(editorPath, QStringList() << file);
        proc.waitForFinished(-1);
    }
}
