/* ========================================================================
*    Copyright (C) 2013-2020 Blaze <blaze@vivaldi.net>
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
    listWidget->setDragDropMode(QAbstractItemView::InternalMove);
    listWidget->setSelectionMode(QAbstractItemView::MultiSelection);
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
    //  populate list widget
    int index = 0;
    for(const BookmarkEntry& entry : qAsConst(list)) {
        auto item = new QListWidgetItem();
        item->setText(tr("Name: ") + entry.name +
                      QChar::fromLatin1('\n') + entry.uri);
        item->setIcon(QIcon::fromTheme(QStringLiteral("audio-x-generic")));
        item->setData(Qt::UserRole, index++);
        listWidget->addItem(item);
    }
}

BookmarkList BookmarkManager::getList() {
    QSettings settings;
    const QString string = settings.value(
                QStringLiteral("bookmarkmanager/bookmarks")).toString();
    const QStringList stringList = string.split(
                QChar::fromLatin1(';'), QString::SkipEmptyParts);
    BookmarkList list;
    list.reserve(stringList.size());
    for(const QString& str : stringList) {
        QStringList bookmark = str.split(QChar::fromLatin1('|'));
        if(bookmark.size() == 2) {
            const BookmarkEntry entry{bookmark.at(0), bookmark.at(1)};
            list.push_back(entry);
        }
    }
    return list;
}

void BookmarkManager::bookmarkCurrent() {
    const QString& url = PLAYER->getTrack().file;
    if(!url.isEmpty() && PLAYER->getTrack().isStream) {
        const QString name = url.split(
                    QChar::fromLatin1('/'), QString::SkipEmptyParts).last();
        QSettings settings;
        QString string = settings.value(
                    QStringLiteral("bookmarkmanager/bookmarks")).toString();
        string.append(name + QChar::fromLatin1('|') +
                      url + QChar::fromLatin1(';'));
        settings.setValue(QStringLiteral("bookmarkmanager/bookmarks"), string);
        if(TrayIcon::self())
            TrayIcon::self()->refreshBookmarks();
    } else { qInfo("invalid bookmark"); }
}

void BookmarkManager::moveUp() {
    int cur = listWidget->currentRow();
    /* Always check if index is valid before usage.
     * QListWidget doesn't care about your selection. */
    if(cur-1 > -1) {
        QListWidgetItem* currentItem = listWidget->takeItem(cur);
        listWidget->insertItem(cur - 1, currentItem);
        listWidget->setCurrentItem(currentItem);
    }
}

void BookmarkManager::moveDown() {
    int cur = listWidget->currentRow();
    if(cur > -1 && cur+1 < listWidget->count()) {
        QListWidgetItem* currentItem = listWidget->takeItem(cur);
        listWidget->insertItem(cur + 1, currentItem);
        listWidget->setCurrentItem(currentItem);
    }
}

void BookmarkManager::deleteBookmark() {
    const QList<QListWidgetItem*>& selection = listWidget->selectedItems();
    for(auto item : selection)
        delete item;
}

void BookmarkManager::renameBookmark(QString name) {
    int index = listWidget->currentItem()->data(Qt::UserRole).toInt();
    list[index].name = name.replace(QChar::fromLatin1(';'), QChar())
            .replace(QChar::fromLatin1('|'), QChar());
    listWidget->currentItem()->setText(tr("Name: ") + name +
                                       QChar::fromLatin1('\n') +
                                       list.at(index).uri);
}

void BookmarkManager::updateLineEdit(int cur) {
    if(cur > -1) {
        int index = listWidget->item(cur)->data(Qt::UserRole).toInt();
        lineEdit->setText(list.at(index).name);
    } else {
        lineEdit->clear();
    }
}

void BookmarkManager::accepted() {
    QSettings settings;
    QString string;
    for(int i = 0; i < listWidget->count(); i++) {
        int index = listWidget->item(i)->data(Qt::UserRole).toInt();
        string.append(list.at(index).name + QChar::fromLatin1('|') +
                      list.at(index).uri + QChar::fromLatin1(';'));
    }
    settings.setValue(QStringLiteral("bookmarkmanager/bookmarks"), string);
    TrayIcon::self()->refreshBookmarks();
    close();
}

void BookmarkManager::copyToClipboard() {
    int index = listWidget->currentItem()->data(Qt::UserRole).toInt();
    QApplication::clipboard()->setText(list.at(index).uri);
}

void BookmarkManager::appendToPlaylist() {
    int index = listWidget->currentItem()->data(Qt::UserRole).toInt();
    PLAYER->appendFile(QStringList{list.at(index).uri});
}
