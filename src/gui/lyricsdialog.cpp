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
    httpObject(new QNetworkAccessManager(this)),
    replyObject(nullptr)
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
    connect(PlayerInterface::self(), &PlayerInterface::newTrack,
            this, [this, autoButton] { if(autoButton->isChecked()) update(); });
    connect(autoButton, &QPushButton::pressed, this, [this] {
        if(artistLineEdit->text() != format(PlayerInterface::getTrack()->artist)
                || titleLineEdit->text() != format(
                    PlayerInterface::getTrack()->title)) update();
    });
    update();
}

void LyricsDialog::showText(QNetworkReply* reply) {
    label->setText(QStringLiteral("OK"));
    if(reply->error() != QNetworkReply::NoError) {
        label->setText(tr("Network error"));
        lyricsBrowser->setText(reply->errorString());
        replyObject = nullptr;
        reply->deleteLater();
        return;
    }
    QString content = QString::fromUtf8(reply->readAll().constData());
    if(replyObject == reply) {
        replyObject = nullptr;
        reply->deleteLater();
        QRegularExpression re(
                    QStringLiteral("<artist>(.*)</artist>.*<song>(.*)</song>"
                                   ".*<lyrics>(.*)</lyrics>.*<url>(.*)</url>"),
                    QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch match = re.match(content);
        if(!match.hasMatch()) {
            lyricsBrowser->setHtml(QLatin1String("<b>")
                                   + tr("Error")
                                   + QLatin1String("</b>"));
            return;
        } else if(match.captured(3) == QStringLiteral("Not found")) {
            lyricsBrowser->setHtml(QLatin1String("<b>")
                                   + tr("Not found")
                                   + QLatin1String("</b>"));
            return;
        } else {
            artistString = match.captured(1);
            titleString = match.captured(2);
        }
        QString urlString = match.captured(4).toLatin1();
        urlString.replace(
                    QLatin1String("http://lyrics.wikia.com/"),
                    QLatin1String("http://lyrics.wikia.com/index.php?title="));
        urlString.append(QLatin1String("&action=edit"));
        QNetworkRequest request;
        request.setUrl(QUrl::fromEncoded(urlString.toLatin1()));
        request.setRawHeader("Referer", match.captured(4).toLatin1());
        label->setText(tr("Downloading"));
        httpObject->get(request);
        reply->deleteLater();
        return;
    }
    QRegularExpression re(QStringLiteral("&lt;lyrics>(.*)&lt;/lyrics>"),
                          QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = re.match(content);
    QString text = QString(QStringLiteral("<h2>%1 - %2</h2>"))
            .arg(artistString, titleString);
    if(match.hasMatch()) {
        QString lyrics = match.captured(1).trimmed();
        lyrics.replace(QLatin1String("\n"), QLatin1String("<br>"));
        text.append(lyrics);
    }
    lyricsBrowser->setHtml(text);
    reply->deleteLater();
}

QString LyricsDialog::format(QString string) {
    return string.replace(QChar::fromLatin1('&'), QLatin1String("and"));
}

void LyricsDialog::update() {
    artistLineEdit->setText(format(PlayerInterface::getTrack()->artist));
    titleLineEdit->setText(format(PlayerInterface::getTrack()->title));
    if(!artistLineEdit->text().isEmpty())
        search();
}

void LyricsDialog::search() {
    label->setText(tr("Searching"));
    setWindowTitle(QString(QStringLiteral("%1 - %2"))
                   .arg(artistLineEdit->text(), titleLineEdit->text()));
    QNetworkRequest request;
    request.setUrl(QUrl(QStringLiteral("http://lyrics.wikia.com/api.php"
                                       "?action=lyrics&artist=")
                        + artistLineEdit->text() + QLatin1String("&song=")
                        + titleLineEdit->text() + QLatin1String("&fmt=xml")));
    request.setRawHeader("User-Agent", "Mozilla/5.0");
    replyObject = httpObject->get(request);
}
