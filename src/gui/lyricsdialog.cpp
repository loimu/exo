/* ========================================================================
*    Copyright (C) 2013-2018 Blaze <blaze@vivaldi.net>
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
#include <QRegExp>
#include <QTimer>

#include "playerinterface.h"
#include "lyricsdialog.h"

LyricsDialog::LyricsDialog(QWidget *parent) : BaseDialog(parent),
    httpObject(new QNetworkAccessManager(this)),
    replyObject(nullptr)
{
    resize(388, 488);
    setWindowTitle(tr("Lyrics"));
    QVBoxLayout* verticalLayout = new QVBoxLayout(this);
    verticalLayout->setContentsMargins(6, -1, 6, 6);
    QHBoxLayout* horizontalLayout = new QHBoxLayout();
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
    QHBoxLayout* horizontalLayout2 = new QHBoxLayout();
    label = new QLabel(this);
    horizontalLayout2->addWidget(label);
    QSpacerItem* spacer = new QSpacerItem(383, 20, QSizePolicy::Expanding,
                                          QSizePolicy::Minimum);
    horizontalLayout2->addItem(spacer);
    QPushButton* prevButton = new QPushButton(this);
    prevButton->setMaximumWidth(23);
    prevButton->setText(QChar::fromLatin1('\253'));
    prevButton->setToolTip(tr("Prev track"));
    horizontalLayout2->addWidget(prevButton);
    QPushButton* nextButton = new QPushButton(this);
    nextButton->setMaximumWidth(23);
    nextButton->setText(QChar::fromLatin1('\273'));
    nextButton->setToolTip(tr("Next track"));
    horizontalLayout2->addWidget(nextButton);
    QPushButton* updateButton = new QPushButton(this);
    updateButton->setText(tr("Update"));
    updateButton->setToolTip(tr("Get lyrics for the current track"));
    updateButton->setFocus();
    horizontalLayout2->addWidget(updateButton);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Close);
    horizontalLayout2->addWidget(buttonBox);
    verticalLayout->addLayout(horizontalLayout2);
    connect(httpObject, &QNetworkAccessManager::finished,
            this, &LyricsDialog::showText);
    connect(artistLineEdit, &QLineEdit::returnPressed,
            this, &LyricsDialog::search);
    connect(titleLineEdit, &QLineEdit::returnPressed,
            this, &LyricsDialog::search);
    connect(prevButton, &QPushButton::released, this, &LyricsDialog::prev);
    connect(nextButton, &QPushButton::released, this, &LyricsDialog::next);
    connect(updateButton, &QPushButton::released, this, &LyricsDialog::update);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &LyricsDialog::close);
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
        QRegExp songRgx(
                    QStringLiteral("<artist>(.*)</artist>.*<song>(.*)</song>.*"
                                   "<lyrics>(.*)</lyrics>.*<url>(.*)</url>"));
        songRgx.setMinimal(true);
        if(songRgx.indexIn(content) < 0) {
            lyricsBrowser->setHtml(QLatin1String("<b>")
                                   + tr("Error")
                                   + QLatin1String("</b>"));
            return;
        }
        else if(songRgx.cap(3) == QStringLiteral("Not found")) {
            lyricsBrowser->setHtml(QLatin1String("<b>")
                                   + tr("Not found")
                                   + QLatin1String("</b>"));
            return;
        }
        else {
            artistString = songRgx.cap(1);
            titleString = songRgx.cap(2);
        }
        QString urlString = songRgx.cap(4).toLatin1();
        urlString.replace(
                    QLatin1String("http://lyrics.wikia.com/"),
                    QLatin1String("http://lyrics.wikia.com/index.php?title="));
        urlString.append(QLatin1String("&action=edit"));
        QNetworkRequest request;
        request.setUrl(QUrl::fromEncoded(urlString.toLatin1()));
        request.setRawHeader("Referer", songRgx.cap(4).toLatin1());
        label->setText(tr("Downloading"));
        httpObject->get(request);
        reply->deleteLater();
        return;
    }
    QRegExp lyricsRgx(QStringLiteral("&lt;lyrics>(.*)&lt;/lyrics>"));
    lyricsRgx.indexIn(content);
    QString text = QString("<h2>%1 - %2</h2>").arg(artistString, titleString);
    QString lyrics = lyricsRgx.cap(1);
    lyrics = lyrics.trimmed();
    lyrics.replace(QLatin1String("\n"), QLatin1String("<br>"));
    text.append(lyrics);
    lyricsBrowser->setHtml(text);
    reply->deleteLater();
}

QString LyricsDialog::format(QString string) {
    string.replace(QLatin1String("&"), QLatin1String("and"));
    return string;
}

void LyricsDialog::update() {
    PlayerInterface* player = PlayerInterface::self();
    artistLineEdit->setText(format(player->trackObject()->artist));
    titleLineEdit->setText(format(player->trackObject()->title));
    if(!artistLineEdit->text().isEmpty())
        search();
}

void LyricsDialog::prev() {
    PlayerInterface::self()->prev();
    lyricsBrowser->setHtml(tr("Please wait a second"));
    QTimer::singleShot(1500, this, [this] { update(); });
}

void LyricsDialog::next() {
    PlayerInterface::self()->next();
    lyricsBrowser->setHtml(tr("Please wait a second"));
    QTimer::singleShot(1500, this, [this] { update(); });
}

void LyricsDialog::search() {
    label->setText(tr("Searching"));
    setWindowTitle(QString("%1 - %2").arg(artistLineEdit->text(),
                                          titleLineEdit->text()));
    QNetworkRequest request;
    request.setUrl(
                QUrl(QLatin1String("http://lyrics.wikia.com/api.php"
                                   "?action=lyrics&artist=")
                     + artistLineEdit->text() + QLatin1String("&song=")
                     + titleLineEdit->text() + QLatin1String("&fmt=xml")));
    request.setRawHeader("User-Agent", "Mozilla/5.0");
    replyObject = httpObject->get(request);
}
