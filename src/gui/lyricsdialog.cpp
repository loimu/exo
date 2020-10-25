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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTextBrowser>
#include <QLabel>
#include <QSpacerItem>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QRegularExpression>
#include <QTimer>

#include "playerinterface.h"
#include "lyricsdialog.h"

LyricsDialog::LyricsDialog(QWidget* parent) : BaseDialog(parent),
    httpObject(new QNetworkAccessManager(this))
{
    resize(388, 488);
    setWindowTitle(tr("Lyrics"));
    auto verticalLayout = new QVBoxLayout(this);
    verticalLayout->setContentsMargins(6, -1, 6, 6);
    auto horizontalLayout = new QHBoxLayout();
    verticalLayout->addLayout(horizontalLayout);
    artistLineEdit = new QLineEdit(this);
    artistLineEdit->setPlaceholderText(tr("artist"));
    artistLineEdit->setToolTip(tr("Edit artist and then press Enter"));
    artistLineEdit->setClearButtonEnabled(true);
    horizontalLayout->addWidget(artistLineEdit);
    titleLineEdit = new QLineEdit(this);
    titleLineEdit->setPlaceholderText(tr("title"));
    titleLineEdit->setToolTip(tr("Edit title and then press Enter"));
    titleLineEdit->setClearButtonEnabled(true);
    horizontalLayout->addWidget(titleLineEdit);
    lyricsBrowser = new QTextBrowser(this);
    lyricsBrowser->setOpenExternalLinks(true);
    verticalLayout->addWidget(lyricsBrowser);
    auto horizontalLayout2 = new QHBoxLayout();
    label = new QLabel(this);
    horizontalLayout2->addWidget(label);
    auto spacer = new QSpacerItem(383, 20, QSizePolicy::Expanding,
                                          QSizePolicy::Minimum);
    horizontalLayout2->addItem(spacer);
    auto autoButton = new QPushButton(this);
    autoButton->setText(tr("Auto"));
    autoButton->setToolTip(tr("Autoupdate track when changed"));
    autoButton->setCheckable(true);
    horizontalLayout2->addWidget(autoButton);
    auto updateButton = new QPushButton(this);
    updateButton->setText(tr("Update"));
    updateButton->setToolTip(tr("Get lyrics for the current track"));
    updateButton->setFocus();
    horizontalLayout2->addWidget(updateButton);
    auto buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Close);
    horizontalLayout2->addWidget(buttonBox);
    verticalLayout->addLayout(horizontalLayout2);
    connect(httpObject, &QNetworkAccessManager::finished,
            this, &LyricsDialog::showText);
    connect(artistLineEdit, &QLineEdit::returnPressed,
            this, &LyricsDialog::search);
    connect(titleLineEdit, &QLineEdit::returnPressed,
            this, &LyricsDialog::search);
    connect(updateButton, &QPushButton::released, this, &LyricsDialog::update);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &LyricsDialog::close);
    connect(PLAYER, &PlayerInterface::newTrack, this, [this, autoButton] {
        if(autoButton->isChecked()) update();
    });
    connect(autoButton, &QPushButton::pressed, this, [this] {
        if(artistLineEdit->text() != format(PLAYER->getTrack().artist)
                || titleLineEdit->text() != format(
                    PLAYER->getTrack().title)) update();
    });
    update();
}

void LyricsDialog::showText(QNetworkReply* reply) {
    label->setText(QStringLiteral("OK"));
    if(reply->error() != QNetworkReply::NoError) {
        label->setText(tr("Network error"));
        lyricsBrowser->setText(reply->errorString());
        reply->deleteLater();
        return;
    }
    const QString content = reply->readAll();
    QRegularExpression re(QStringLiteral("<p>(.*)</p>"),
                          QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = re.match(content);
    if(match.hasMatch())
        lyricsBrowser->setHtml(match.captured(1).trimmed());
    reply->deleteLater();
}

QString LyricsDialog::format(QString string) {
    return string.replace(QChar::fromLatin1('&'), QLatin1String("and"));
}

QString LyricsDialog::replace(QString string) {
    const QString rep = QString::fromLatin1(" _@,;&\"");
    for(const QChar& c: rep)
        string = string.replace(c, QChar::fromLatin1('_'));
    return string.replace(QChar::fromLatin1('.'), QChar());
}

void LyricsDialog::update() {
    artistLineEdit->setText(format(PLAYER->getTrack().artist));
    titleLineEdit->setText(format(PLAYER->getTrack().title));
    if(!artistLineEdit->text().isEmpty())
        search();
}

void LyricsDialog::search() {
    label->setText(tr("Fetching"));
    setWindowTitle(QString(QStringLiteral("%1 - %2"))
                   .arg(artistLineEdit->text(), titleLineEdit->text()));
    QNetworkRequest request;
    request.setUrl(QUrl(QLatin1String("http://www.lyriki.com/")
                        + replace(artistLineEdit->text()) + QChar::fromLatin1(':')
                        + replace(titleLineEdit->text())));
    request.setRawHeader("Accept",
                         "text/html,application/xhtml+xml,application/xml;q=0.9,"
                         "image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9");
    request.setRawHeader("User-Agent", "Mozilla/5.0");
    httpObject->get(request);
}
