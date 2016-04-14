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

#include <QSettings>
#include <QStringList>
#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLineEdit>

#include "../playerinterface.h"
#include "bookmarkmanager.h"
#include "bookmarkdialog.h"

// maximal amount of bookmarks
#define MAX_SIZE 10

BookmarkManager::BookmarkManager(QObject *parent) : QObject(parent)
{
    refreshList();
}

BookmarkManager::~BookmarkManager()
{
    if(bookmarkManager)
        bookmarkManager->deleteLater();
}

void BookmarkManager::refreshList() {
    list.clear();
    QSettings settings;
    QString string = settings.value("bookmarkmanager/bookmarks").toString();
    QStringList stringList = string.split(";");
    if(stringList.size() > 0) {
        foreach (QString str, stringList) {
            QStringList bookmark = str.split("|");
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
    BookmarkEntry entry;
    entry.uri  = PlayerInterface::instance()->trackObject()->file;
    entry.name = entry.uri;
    if(entry.uri.isEmpty() || list.size() >= MAX_SIZE)
        return;
    list.append(entry);
    save();
}

void BookmarkManager::manager() {
    refreshList();
    bookmarkManager = new BookmarkDialog(static_cast<QWidget*>(this->parent()), &list);
    connect(bookmarkManager, SIGNAL(save()), SLOT(save()));
    bookmarkManager->show();
}

void BookmarkManager::save() {
    QSettings settings;
    QString string = QString();
    int count = 0;
    foreach(BookmarkEntry entry, list) {
        if(count)
            string.append(";");
        string.append(entry.name + "|" + entry.uri);
        count++;
    }
    settings.setValue("bookmarkmanager/bookmarks", string);
    emit refreshBookmarks();
}
