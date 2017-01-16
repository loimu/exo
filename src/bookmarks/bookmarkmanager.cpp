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

#include <QSettings>

#include "core/playerinterface.h"
#include "gui/trayicon.h"
#include "bookmarkdialog.h"
#include "bookmarkmanager.h"

BookmarkManager::BookmarkManager(QObject *parent) : QObject(parent)
{
    refreshList();
}

void BookmarkManager::refreshList() {
    list.clear();
    QSettings settings;
    QString string = settings.value(
                QLatin1String("bookmarkmanager/bookmarks")).toString();
    QStringList stringList = string.split(QLatin1String(";"));
    if(!stringList.isEmpty()) {
        for(QString str : stringList) {
            QStringList bookmark = str.split(QLatin1String("|"));
            if(bookmark.size() == 2) {
                BookmarkEntry entry;
                entry.name = bookmark.at(0);
                entry.uri = bookmark.at(1);
                list.append(entry);
            }
        }
    }
}

QList<BookmarkEntry>* BookmarkManager::bookmarks() {
    return &list;
}

void BookmarkManager::addCurrent() {
    if(bookmarkDialog)
        return;
    BookmarkEntry entry;
    entry.uri  = PlayerInterface::self()->trackObject()->file;
    entry.name = entry.uri;
    if(entry.uri.isEmpty())
        return;
    list.append(entry);
    save();
}

void BookmarkManager::manager() {
    if(bookmarkDialog)
        return;
    refreshList();
    bookmarkDialog = new BookmarkDialog(&list, TrayIcon::self());
    connect(bookmarkDialog, SIGNAL(save()), SLOT(save()));
    bookmarkDialog->show();
}

void BookmarkManager::save() {
    QSettings settings;
    QString string = QString();
    int count = 0;
    for(BookmarkEntry entry : list) {
        if(count)
            string.append(QLatin1String(";"));
        string.append(entry.name + QLatin1String("|") + entry.uri);
        count++;
    }
    settings.setValue(QLatin1String("bookmarkmanager/bookmarks"), string);
    emit refreshBookmarks();
}
