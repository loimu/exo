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

#ifndef BOOKMARKMANAGER_H
#define BOOKMARKMANAGER_H

#include <QObject>
#include <QVector>
#include <QPointer>

class BookmarkDialog;

struct BookmarkEntry {
    QString name;
    QString uri;
};

class BookmarkManager : public QObject
{
    Q_OBJECT

    QVector<BookmarkEntry> list;
    QPointer<BookmarkDialog> bookmarkDialog;
    void refreshList();

public:
    explicit BookmarkManager(QObject *parent = nullptr);
    QVector<BookmarkEntry>* bookmarks();

public slots:
    void addCurrent();
    void manager();
    void save();

signals:
    void refreshBookmarks();
};

#endif // BOOKMARKMANAGER_H
