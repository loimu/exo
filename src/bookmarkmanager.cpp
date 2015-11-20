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

#include <QSettings>
#include <QStringList>
#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDialogButtonBox>

#include "playerinterface.h"
#include "bookmarkmanager.h"

// maximal amount of bookmarks
#define MAX_SIZE 10

BookmarkManager::BookmarkManager(QObject *parent) : QObject(parent)
{
    QSettings settings;
    for(int i=0; i<MAX_SIZE; i++) {
        BookmarkEntry entry;
        entry.name = settings.value(QString("bookmarks/name%1").arg(i)).toString();
        entry.uri = settings.value(QString("bookmarks/link%1").arg(i)).toString();
        if(!entry.uri.isEmpty())
            list.append(entry);
    }
}

void BookmarkManager::save() {
    QSettings settings;
    int count = 0;
    foreach(BookmarkEntry entry, list) {
        settings.setValue(QString("bookmarks/name%1").arg(count), entry.name);
        settings.setValue(QString("bookmarks/link%1").arg(count), entry.uri);
        count++;
    }
}

QStringList BookmarkManager::bookmarks() {
    QStringList res;
    foreach(BookmarkEntry entry, list)
        res.append(entry.name);
    return res;
}

void BookmarkManager::addCurrent() {
    BookmarkEntry entry;
    entry.name = PlayerInterface::instance()->trackObject()->title;
    entry.uri = PlayerInterface::instance()->trackObject()->file;
    if(entry.uri.isEmpty() || list.size() >= MAX_SIZE)
        return;
    list.append(entry);
    save();
}

void BookmarkManager::manager() {
    QDialog *bookmarkManager = new QDialog();
    bookmarkManager->setWindowTitle(tr("Bookmark Manager"));
    bookmarkManager->setModal(false);
    bookmarkManager->resize(500,250);
    QVBoxLayout *verticalLayout = new QVBoxLayout(bookmarkManager);
    QListWidget *listWidget = new QListWidget(bookmarkManager);
    verticalLayout->addWidget(listWidget);
    QHBoxLayout *horizontalLayout = new QHBoxLayout();
    QPushButton *deleteButton = new QPushButton(bookmarkManager);
    deleteButton->setText(tr("&Delete"));
    horizontalLayout->addWidget(deleteButton);
    QSpacerItem *horizontalSpacer = new QSpacerItem(40,20,QSizePolicy::Expanding,
                                                    QSizePolicy::Minimum);
    horizontalLayout->addItem(horizontalSpacer);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(bookmarkManager);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    horizontalLayout->addWidget(buttonBox);
    verticalLayout->addLayout(horizontalLayout);
    connect(buttonBox, SIGNAL(rejected()), bookmarkManager, SLOT(close()));
    bookmarkManager->show();
    save();
}
