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


#ifndef BOOKMARKDIALOG_H
#define BOOKMARKDIALOG_H

#include <QWidget>
#include <QList>

class QLineEdit;
class QListWidget;
class BookmarkEntry;

class BookmarkDialog : public QWidget
{
    Q_OBJECT

    QLineEdit* lineEdit;
    QListWidget* listWidget;
    QList<BookmarkEntry>* list_;
    void keyPressEvent(QKeyEvent *e);

public:
    explicit BookmarkDialog(QWidget *parent = nullptr,
                            QList<BookmarkEntry> *list = nullptr);
    void refreshView();

signals:
    void save();

private slots:
    void deleteBookmark();
    void renameBookmark(QString);
    void updateLineEdit(int cur);
    void accepted();
};

#endif // BOOKMARKDIALOG_H
