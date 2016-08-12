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

#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QKeyEvent>

#include "bookmarkmanager.h"
#include "bookmarkdialog.h"

BookmarkDialog::BookmarkDialog(QWidget *parent, QList<BookmarkEntry> *list) : QWidget(parent),
    list_(list),
    list(*list) // use local copy of the list
{
    setWindowFlags(Qt::Dialog);
    setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowTitle(tr("Bookmark Manager"));
    this->resize(500, 550);
    QVBoxLayout *verticalLayout = new QVBoxLayout(this);
    listWidget = new QListWidget(this);
    listWidget->setIconSize(QSize(32, 32));
    verticalLayout->addWidget(listWidget);
    QHBoxLayout *horizontalLayout = new QHBoxLayout();
    QPushButton *deleteButton = new QPushButton(this);
    deleteButton->setText(tr("&Delete"));
    deleteButton->setToolTip(tr("Delete selected item"));
    deleteButton->setIcon(QIcon::fromTheme(QLatin1String("edit-delete")));
    horizontalLayout->addWidget(deleteButton);
    lineEdit = new QLineEdit(this);
    lineEdit->setToolTip(tr("Rename selected item"));
    lineEdit->setPlaceholderText("Rename");
    horizontalLayout->addWidget(lineEdit);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    horizontalLayout->addWidget(buttonBox);
    verticalLayout->addLayout(horizontalLayout);
    connect(listWidget, SIGNAL(currentRowChanged(int)), SLOT(updateLineEdit(int)));
    connect(deleteButton, SIGNAL(released()), SLOT(deleteBookmark()));
    connect(lineEdit, SIGNAL(textEdited(QString)), SLOT(renameBookmark(QString)));
    connect(buttonBox, SIGNAL(accepted()), SLOT(accepted()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));
    refreshView();
}

void BookmarkDialog::refreshView() {
    lineEdit->clear();
    listWidget->clear();
    for(BookmarkEntry entry : list) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setText(tr("Name: ") + entry.name + "\n" + entry.uri);
        item->setIcon(QIcon::fromTheme(QLatin1String("audio-x-generic")));
        listWidget->addItem(item);
    }
}

void BookmarkDialog::deleteBookmark() {
    int cur = listWidget->currentRow();
    if(cur > -1) {
        list.removeAt(cur);
        refreshView();
    }
}

void BookmarkDialog::renameBookmark(QString name) {
    int cur = listWidget->currentRow();
    // always check if index is valid before usage
    if(cur > -1) {
        list[cur].name = name.replace(";", "").replace("|", "");
        listWidget->currentItem()->setText(tr("Name: ") + name + "\n"
                                           + list.at(cur).uri);
    }
}

void BookmarkDialog::updateLineEdit(int cur) {
    if(cur > -1)
        lineEdit->setText(list.at(cur).name);
}

void BookmarkDialog::accepted() {
    *list_ = list;
    emit save();
    this->close();
}

void BookmarkDialog::keyPressEvent(QKeyEvent *e) {
    if(e->key() == Qt::Key_Escape)
        this->close();
    if(e->key() == Qt::Key_Delete)
        this->deleteBookmark();
}
