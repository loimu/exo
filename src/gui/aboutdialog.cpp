/* ========================================================================
*    Copyright (C) 2013-2017 Blaze <blaze@vivaldi.net>
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

#include <QApplication>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) : BaseDialog(parent)
{
    this->setWindowTitle(tr("About eXo"));
    QHBoxLayout* horizontalLayout = new QHBoxLayout(this);
    QLabel* label = new QLabel(this);
    label->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    horizontalLayout->addWidget(label);
    QVBoxLayout* verticalLayout = new QVBoxLayout();
    verticalLayout->setSpacing(6);
    verticalLayout->setSizeConstraint(QLayout::SetMaximumSize);
    QLabel* iconLabel = new QLabel(this);
    QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(iconLabel->sizePolicy().hasHeightForWidth());
    iconLabel->setSizePolicy(sizePolicy);
    iconLabel->setPixmap(QPixmap(QLatin1String(":/images/128.png")));
    iconLabel->setAlignment(Qt::AlignRight|Qt::AlignTop|Qt::AlignTrailing);
    verticalLayout->addWidget(iconLabel);
    QPushButton* aboutQt = new QPushButton(this);
    aboutQt->setText(tr("About &Qt"));
    connect(aboutQt, SIGNAL(released()), qApp, SLOT(aboutQt()));
    verticalLayout->addWidget(aboutQt);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
    buttonBox->setMaximumSize(QSize(128, 16777215));
    buttonBox->setOrientation(Qt::Vertical);
    buttonBox->setStandardButtons(QDialogButtonBox::Close);
    buttonBox->setCenterButtons(false);
    verticalLayout->addWidget(buttonBox);
    horizontalLayout->addLayout(verticalLayout);
    connect(buttonBox, SIGNAL(rejected()), SLOT(close()));
    label->setText(QString("<p><b>eXo</b> v%1 &copy; 2013-2017 Blaze<br />"
                        "&lt;blaze@vivaldi.net&gt;</p>"
                        "<p>Qt %2 (built with Qt %3)<br />"
                        "Licensed under GPL v3 or later.</p>"
                        "<p><b>Links:</b><br />"
                        "Project: <a href=\"https://bitbucket.org/blaze/exo\">"
                        "https://bitbucket.org/blaze/exo</a><br />"
                        "Patreon: <a href=\"https://patreon.com/blazy\">"
                        "https://patreon.com/blazy</a></p>")
                .arg(qApp->applicationVersion())
                .arg(qVersion())
                .arg(QT_VERSION_STR));
    layout()->setSizeConstraint(QLayout::SetFixedSize);
}
