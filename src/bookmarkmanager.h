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

#ifndef BOOKMARKMANAGER_H
#define BOOKMARKMANAGER_H

#include <QObject>
#include <QList>
#include <QPointer>

class QDialog;
class QListWidget;

struct BookmarkEntry {
    QString name;
    QString uri;
};

class BookmarkManager : public QObject
{
    Q_OBJECT

    QList<BookmarkEntry> list;
    QPointer<QDialog> bookmarkManager;
    QPointer<QListWidget> listWidget;
    void refresh();

public:
    explicit BookmarkManager(QObject *parent = 0);
    ~BookmarkManager();
    QList<BookmarkEntry> bookmarks();

public slots:
    void addCurrent();
    void manager();

signals:
    void refreshBookmarks();

private slots:
    void save();
    void deleteBookmark();
    void renameBookmark();
};

#endif // BOOKMARKMANAGER_H
