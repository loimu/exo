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

#include "playerinterface.h"
#include "bookmarkmanager.h"

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

void BookmarkManager::refreshView() {
    listWidget->clear();
    foreach(BookmarkEntry entry, list) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setText(tr("Name: ") + entry.name + "\n" + tr("URI: ") + entry.uri);
        listWidget->addItem(item);
    }
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

QList<BookmarkEntry> BookmarkManager::bookmarks() {
    return list;
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
    bookmarkManager = new QDialog();
    bookmarkManager->setWindowTitle(tr("Bookmark Manager"));
    bookmarkManager->setModal(false);
    bookmarkManager->resize(500,250);
    QVBoxLayout *verticalLayout = new QVBoxLayout(bookmarkManager);
    listWidget = new QListWidget(bookmarkManager);
    verticalLayout->addWidget(listWidget);
    refreshView();
    QHBoxLayout *horizontalLayout = new QHBoxLayout();
    lineEdit = new QLineEdit(bookmarkManager);
    horizontalLayout->addWidget(lineEdit);
    lineEdit->setVisible(false);
    lineEdit->setText(list[0].name);
    QPushButton *acceptButton = new QPushButton(bookmarkManager);
    acceptButton->setText(tr("Accept"));
    acceptButton->setVisible(false);
    horizontalLayout->addWidget(acceptButton);
    QHBoxLayout *horizontalLayout2 = new QHBoxLayout();
    QPushButton *deleteButton = new QPushButton(bookmarkManager);
    deleteButton->setText(tr("&Delete"));
    horizontalLayout2->addWidget(deleteButton);
    QPushButton *renameButton = new QPushButton(bookmarkManager);
    renameButton->setText(tr("&Rename"));
    horizontalLayout2->addWidget(renameButton);
    QSpacerItem *horizontalSpacer = new QSpacerItem(40,20,QSizePolicy::Expanding,
                                                    QSizePolicy::Minimum);
    horizontalLayout2->addItem(horizontalSpacer);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(bookmarkManager);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    horizontalLayout2->addWidget(buttonBox);
    verticalLayout->addLayout(horizontalLayout);
    verticalLayout->addLayout(horizontalLayout2);
    connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(updateLineEdit()));
    connect(acceptButton, SIGNAL(released()), SLOT(renameBookmark()));
    connect(acceptButton, SIGNAL(released()), lineEdit, SLOT(hide()));
    connect(acceptButton, SIGNAL(released()), acceptButton, SLOT(hide()));
    connect(deleteButton, SIGNAL(released()), SLOT(deleteBookmark()));
    connect(renameButton, SIGNAL(released()), acceptButton, SLOT(show()));
    connect(renameButton, SIGNAL(released()), lineEdit, SLOT(show()));
    connect(buttonBox, SIGNAL(accepted()), SLOT(save()));
    connect(buttonBox, SIGNAL(accepted()), bookmarkManager, SLOT(close()));
    connect(buttonBox, SIGNAL(rejected()), bookmarkManager, SLOT(close()));
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

void BookmarkManager::deleteBookmark() {
    list.removeAt(listWidget->currentRow());
    refreshView();
}

void BookmarkManager::renameBookmark() {
    list[listWidget->currentRow()].name = lineEdit->text();
    refreshView();
    lineEdit->clear();
}

void BookmarkManager::updateLineEdit() {
    lineEdit->setText(list[listWidget->currentRow()].name);
}
