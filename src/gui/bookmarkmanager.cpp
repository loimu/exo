/* ========================================================================
*    Copyright (C) 2013-2019 Blaze <blaze@vivaldi.net>
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

#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QSettings>
#include <QApplication>
#include <QClipboard>
#include <QMenu>

#include "playerinterface.h"
#include "trayicon.h"
#include "bookmarkmanager.h"


BookmarkManager::BookmarkManager(QWidget* parent) : BaseDialog(parent),
    list(BookmarkManager::getList())
{
    this->setWindowTitle(tr("Bookmark Manager"));
    this->resize(500, 550);
    auto verticalLayout = new QVBoxLayout(this);
    listWidget = new QListWidget(this);
    listWidget->setIconSize(QSize(32, 32));
    verticalLayout->addWidget(listWidget);
    auto horizontalLayout = new QHBoxLayout();
    auto deleteButton = new QPushButton(this);
    deleteButton->setText(tr("&Delete"));
    deleteButton->setToolTip(tr("Delete selected item"));
    deleteButton->setIcon(QIcon::fromTheme(QLatin1String("edit-delete")));
    deleteButton->setShortcut(Qt::Key_Delete);
    horizontalLayout->addWidget(deleteButton);
    lineEdit = new QLineEdit(this);
    lineEdit->setToolTip(tr("Rename selected item"));
    lineEdit->setPlaceholderText(QStringLiteral("Rename"));
    lineEdit->setClearButtonEnabled(true);
    horizontalLayout->addWidget(lineEdit);
    auto buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(
                QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    horizontalLayout->addWidget(buttonBox);
    verticalLayout->addLayout(horizontalLayout);
    auto copyAction = new QAction(tr("Copy URL to clipboard"), this);
    copyAction->setShortcut(Qt::CTRL + Qt::Key_C);
    listWidget->addAction(copyAction);
    auto moveUpAction = new QAction(tr("Move up"), this);
    moveUpAction->setShortcut(Qt::CTRL + Qt::Key_K);
    listWidget->addAction(moveUpAction);
    auto moveDownAction = new QAction(tr("Move down"), this);
    moveDownAction->setShortcut(Qt::CTRL + Qt::Key_J);
    listWidget->addAction(moveDownAction);
    listWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
    connect(listWidget, &QListWidget::currentRowChanged,
            this, &BookmarkManager::updateLineEdit);
    connect(deleteButton, &QPushButton::released,
            this, &BookmarkManager::deleteBookmark);
    connect(lineEdit, &QLineEdit::textEdited,
            this, &BookmarkManager::renameBookmark);
    connect(buttonBox, &QDialogButtonBox::accepted,
            this, &BookmarkManager::accepted);
    connect(buttonBox, &QDialogButtonBox::rejected,
            this, &BookmarkManager::close);
    connect(copyAction, &QAction::triggered,
            this, &BookmarkManager::copyToClipboard);
    connect(moveUpAction, &QAction::triggered, this, &BookmarkManager::moveUp);
    connect(moveDownAction, &QAction::triggered,
            this, &BookmarkManager::moveDown);
    connect(listWidget, &QListWidget::activated,
            this, &BookmarkManager::appendToPlaylist);
    refreshView();
}

BookmarkList BookmarkManager::getList() {
    QSettings settings;
    QString string = settings.value(
                QStringLiteral("bookmarkmanager/bookmarks")).toString();
    QStringList stringList = string.split(
                QChar::fromLatin1(';'), QString::SkipEmptyParts);
    BookmarkList list;
    for(const QString& str: stringList) {
        QStringList bookmark = str.split(QChar::fromLatin1('|'));
        if(bookmark.size() == 2) {
            BookmarkEntry entry{bookmark.at(0), bookmark.at(1)};
            list.append(entry);
        }
    }
    return list;
}

void BookmarkManager::addBookmark(BookmarkList& list, const QString& url) {
    if(!url.isEmpty()) {
        QString name = url.split(
                    QChar::fromLatin1('/'), QString::SkipEmptyParts).last();
        BookmarkEntry entry{name, url};
        list.append(entry);
        BookmarkManager::saveList(list);
    }
}

void BookmarkManager::saveList(const BookmarkList& list) {
    QSettings settings;
    QString string;
    for(const BookmarkEntry& entry: list) {
        string.append(entry.name + QChar::fromLatin1('|') + entry.uri);
        string.append(QChar::fromLatin1(';'));
    }
    settings.setValue(QStringLiteral("bookmarkmanager/bookmarks"), string);
}

void BookmarkManager::refreshView() {
    lineEdit->clear();
    listWidget->clear();
    for(BookmarkEntry& entry : list) {
        auto item = new QListWidgetItem();
        item->setText(tr("Name: ") + entry.name
                      + QLatin1String("\n") + entry.uri);
        item->setIcon(QIcon::fromTheme(QStringLiteral("audio-x-generic")));
        listWidget->addItem(item);
    }
}

void BookmarkManager::moveUp() {
    int cur = listWidget->currentRow();
    /* Always check if index is valid before usage.
     * QListWidget doesn't care about your selection. */
    if(cur-1 > -1) {
        qSwap(list[cur], list[cur-1]);
        refreshView();
        listWidget->setCurrentRow(cur-1);
    }
}

void BookmarkManager::moveDown() {
    int cur = listWidget->currentRow();
    if(cur > -1 && cur+1 < list.size()) {
        qSwap(list[cur], list[cur+1]);
        refreshView();
        listWidget->setCurrentRow(cur+1);
    }
}

void BookmarkManager::deleteBookmark() {
    int cur = listWidget->currentRow();
    if(cur > -1) {
        list.remove(cur);
        refreshView();
    }
}

void BookmarkManager::renameBookmark(QString name) {
    int cur = listWidget->currentRow();
    if(cur > -1) {
        list[cur].name = name.replace(QChar::fromLatin1(';'), QString())
                .replace(QChar::fromLatin1('|'), QString());
        listWidget->currentItem()->setText(tr("Name: ") + name
                                           + QLatin1String("\n")
                                           + list.at(cur).uri);
    }
}

void BookmarkManager::updateLineEdit(int cur) {
    if(cur > -1)
        lineEdit->setText(list.at(cur).name);
}

void BookmarkManager::accepted() {
    BookmarkManager::saveList(list);
    TrayIcon::self()->refreshBookmarks(list);
    close();
}

void BookmarkManager::copyToClipboard() {
    int cur = listWidget->currentRow();
    if(cur > -1)
        QApplication::clipboard()->setText(list.at(cur).uri);
}

void BookmarkManager::appendToPlaylist() {
    int cur = listWidget->currentRow();
    if(cur > -1)
        PLAYER->appendFile(QStringList{list.at(cur).uri});
}
