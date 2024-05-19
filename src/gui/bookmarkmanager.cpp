/* ========================================================================
*    Copyright (C) 2013-2024 Blaze <blaze@vivaldi.net>
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

#include <QVector>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QSettings>
#include <QApplication>
#include <QClipboard>
#include <QAction>

#include "playerinterface.h"
#include "trayicon.h"
#include "bookmarkmanager.h"


BookmarkManager::BookmarkManager(QWidget* parent) : BaseDialog(parent) {
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
    copyAction->setShortcut(Qt::CTRL | Qt::Key_C);
    listWidget->addAction(copyAction);
    auto moveUpAction = new QAction(tr("Move up"), this);
    moveUpAction->setShortcut(Qt::CTRL | Qt::Key_K);
    listWidget->addAction(moveUpAction);
    auto moveDownAction = new QAction(tr("Move down"), this);
    moveDownAction->setShortcut(Qt::CTRL | Qt::Key_J);
    listWidget->addAction(moveDownAction);
    listWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
    listWidget->setDragDropMode(QAbstractItemView::InternalMove);
    listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
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
    // populate list widget
    QSettings settings;
    const QList<QVariant> list =
        settings.value(QStringLiteral("bookmarkmanager/bookmarks2")).toList();
    for(const auto& bookmark : list) {
        auto item = new QListWidgetItem();
        item->setText(tr("Name: ") + bookmark.toStringList().at(0) +
                      QChar::fromLatin1('\n') + bookmark.toStringList().at(1));
        item->setIcon(QIcon::fromTheme(QStringLiteral("audio-x-generic")));
        item->setData(Qt::UserRole, bookmark.toStringList());
        listWidget->addItem(item);
    }
}

void BookmarkManager::migrateBookmarks() {
    QSettings settings;
    const QString string = settings.value(QStringLiteral("bookmarkmanager/bookmarks")).toString();
    if(string.isEmpty()) return;
    const QStringList stringList = string.split(QChar::fromLatin1(';'), Qt::SkipEmptyParts);
    QList<QVariant> list{};
    for(const QString& str : stringList) {
        QStringList bookmark = str.split(QChar::fromLatin1('|'));
        if(bookmark.size() == 2) {
            list.append(bookmark);
        }
    }
    settings.setValue(QStringLiteral("bookmarkmanager/bookmarks2"), list);
    settings.setValue(QStringLiteral("bookmarkmanager/bookmarks"), QString());
}

void BookmarkManager::moveUp() {
    int cur = listWidget->currentRow();
    /* Always check if index is valid before usage.
     * QListWidget doesn't care about your selection. */
    if(cur > 0) {
        QListWidgetItem* currentItem = listWidget->takeItem(cur);
        listWidget->insertItem(cur - 1, currentItem);
        listWidget->setCurrentItem(currentItem);
    }
}

void BookmarkManager::moveDown() {
    int cur = listWidget->currentRow();
    if(cur > -1 && cur + 1 < listWidget->count()) {
        QListWidgetItem* currentItem = listWidget->takeItem(cur);
        listWidget->insertItem(cur + 1, currentItem);
        listWidget->setCurrentItem(currentItem);
    }
}

void BookmarkManager::deleteBookmark() {
    const QList<QListWidgetItem*>& selection = listWidget->selectedItems();
    for(auto item : selection) {
        delete item;
    }
}

void BookmarkManager::renameBookmark(const QString name) {
    const QString uri = listWidget->currentItem()->data(Qt::UserRole).toStringList().at(1);
    listWidget->currentItem()->setData(Qt::UserRole, QStringList{name, uri});
    listWidget->currentItem()->setText(tr("Name: ") + name + QChar::fromLatin1('\n') + uri);
}

void BookmarkManager::updateLineEdit(int cur) {
    if(cur > -1) {
        lineEdit->setText(listWidget->item(cur)->data(Qt::UserRole).toStringList().at(0));
    } else {
        lineEdit->clear();
    }
}

void BookmarkManager::accepted() {
    QSettings settings;
    QList<QVariant> list{};
    for(int i = 0; i < listWidget->count(); i++) {
        const auto& itemData = listWidget->item(i)->data(Qt::UserRole);
        list.append(QStringList{itemData.toStringList().at(0), itemData.toStringList().at(1)});
    }
    settings.setValue(QStringLiteral("bookmarkmanager/bookmarks2"), list);
    TrayIcon::self()->refreshBookmarks(list);
    close();
}

void BookmarkManager::copyToClipboard() {
    QApplication::clipboard()->setText(
        listWidget->currentItem()->data(Qt::UserRole).toStringList().at(1));
}

void BookmarkManager::appendToPlaylist() {
    PLAYER->appendFile(
        QStringList{listWidget->currentItem()->data(Qt::UserRole).toStringList().at(1)});
}
