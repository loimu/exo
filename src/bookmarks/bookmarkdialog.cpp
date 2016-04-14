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
    list_(list)
{
    setWindowFlags(Qt::Dialog);
    setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowTitle(tr("Bookmark Manager"));
    this->resize(500,250);
    QVBoxLayout *verticalLayout = new QVBoxLayout(this);
    listWidget = new QListWidget(this);
    verticalLayout->addWidget(listWidget);
    refreshView();
    QHBoxLayout *horizontalLayout = new QHBoxLayout();
    lineEdit = new QLineEdit(this);
    horizontalLayout->addWidget(lineEdit);
    lineEdit->setVisible(false);
    QPushButton *acceptButton = new QPushButton(this);
    acceptButton->setText(tr("Accept"));
    acceptButton->setVisible(false);
    horizontalLayout->addWidget(acceptButton);
    QHBoxLayout *horizontalLayout2 = new QHBoxLayout();
    QPushButton *deleteButton = new QPushButton(this);
    deleteButton->setText(tr("&Delete"));
    horizontalLayout2->addWidget(deleteButton);
    QPushButton *renameButton = new QPushButton(this);
    renameButton->setText(tr("&Rename"));
    horizontalLayout2->addWidget(renameButton);
    QSpacerItem *horizontalSpacer = new QSpacerItem(40,20,QSizePolicy::Expanding,
                                                    QSizePolicy::Minimum);
    horizontalLayout2->addItem(horizontalSpacer);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
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
    connect(buttonBox, SIGNAL(accepted()), SLOT(accepted()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));
}

void BookmarkDialog::refreshView() {
    listWidget->clear();
    foreach(BookmarkEntry entry, *list_) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setText(tr("Name: ") + entry.name + "\n" + tr("URI: ") + entry.uri);
        listWidget->addItem(item);
    }
}

void BookmarkDialog::deleteBookmark() {
    list_->removeAt(listWidget->currentRow());
    refreshView();
}

void BookmarkDialog::renameBookmark() {
    (*list_)[listWidget->currentRow()].name = lineEdit->text();
    refreshView();
    lineEdit->clear();
}

void BookmarkDialog::updateLineEdit() {
    lineEdit->setText((*list_)[listWidget->currentRow()].name);
}

void BookmarkDialog::accepted() {
    emit save();
    this->close();
}

void BookmarkDialog::keyPressEvent(QKeyEvent *e) {
    if(e->key() == Qt::Key_Escape)
        this->close();
}
