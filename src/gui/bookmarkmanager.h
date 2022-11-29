/* ========================================================================
*    Copyright (C) 2013-2022 Blaze <blaze@vivaldi.net>
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

#include "basedialog.h"

class QLineEdit;
class QListWidget;

struct BookmarkEntry {
    QString name;
    QString uri;
};
template <typename>
class QVector;

using BookmarkList = QVector<BookmarkEntry>;

class BookmarkManager : public BaseDialog
{
    Q_OBJECT

    QLineEdit* lineEdit;
    QListWidget* listWidget;
    BookmarkList list;
    void moveUp();
    void moveDown();
    void deleteBookmark();
    void renameBookmark(QString);
    void updateLineEdit(int cur);
    void accepted();
    void copyToClipboard();
    void appendToPlaylist();

public:
    static BookmarkList getList();
    static void bookmarkCurrent();
    explicit BookmarkManager(QWidget* parent = nullptr);
};

#endif // BOOKMARKMANAGER_H
