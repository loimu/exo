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
#include <QTimer>
#include <QAction>

#include "playerinterface.h"
#include "lyricsproviders.h"
#include "lyricsdialog.h"

#define QSL QStringLiteral


const char* UserAgent = "Mozilla/5.0 (X11; Linux x86_64; rv:125.0) Gecko/20100101 Firefox/125.0";

LyricsDialog::LyricsDialog(int providerNum_, QWidget* parent)
    : BaseDialog(parent)
    , httpObject(new QNetworkAccessManager(this))
    , replyObject(nullptr)
    , providerNum(providerNum_)
{
    resize(388, 488);
    setWindowTitle(tr("Lyrics"));
    auto* verticalLayout = new QVBoxLayout(this);
    verticalLayout->setContentsMargins(6, -1, 6, 6);
    auto* horizontalLayout = new QHBoxLayout();
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
    auto* horizontalLayout2 = new QHBoxLayout();
    label = new QLabel(this);
    horizontalLayout2->addWidget(label);
    auto* spacer =
        new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);
    horizontalLayout2->addItem(spacer);
    auto* autoButton = new QPushButton(this);
    autoButton->setText(tr("Auto"));
    autoButton->setToolTip(tr("Autoupdate track when changed"));
    autoButton->setCheckable(true);
    horizontalLayout2->addWidget(autoButton);
    auto* updateButton = new QPushButton(this);
    updateButton->setText(tr("Update"));
    updateButton->setToolTip(tr("Get lyrics for the current track"));
    updateButton->setFocus();
    horizontalLayout2->addWidget(updateButton);
    auto* buttonBox = new QDialogButtonBox(this);
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
    connect(PLAYER, &PlayerInterface::newTrack, this,
            [this, autoButton] { if(autoButton->isChecked()) update(); });
    connect(autoButton, &QPushButton::pressed, this, [this] {
        if(artistLineEdit->text() != PLAYER->getTrack().artist
            || titleLineEdit->text() != PLAYER->getTrack().title) update();
    });
    update();
}

void LyricsDialog::showText(QNetworkReply* reply) {
    if(reply->error() != QNetworkReply::NoError) {
        label->setText(tr("Error"));
        lyricsBrowser->setHtml(QSL("<b>") + tr("Page not available") + QSL("</b>"));
        replyObject = nullptr;
        reply->deleteLater();
        if(cycleThroughProviders) {
            checkNextProvider();
        }
        return;
    } else {
        label->setText(QStringLiteral("OK"));
    }

    const LyricsProviders::Provider& provider = LyricsProviders::providers[providerNum];
    if(replyObject == reply) {
        replyObject = nullptr;
        QString content = QString::fromUtf8(reply->readAll().constData());
        reply->deleteLater();
        const QRegularExpression urlRegex(
            provider.urlRegex.contains(QSL("%1"))
                ? provider.urlRegex.arg(escapeRegexInput(artistLineEdit->text()))
                : provider.urlRegex, QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch match = urlRegex.match(content);
        if(!match.hasMatch()) {
            lyricsBrowser->setHtml(QSL("<b>") + tr("Not found") + QSL("</b>"));
            reply->deleteLater();
            if(cycleThroughProviders) {
                checkNextProvider();
            }
            return;
        }
        QString urlString = provider.urlTemplate.arg(match.captured(1));
        QNetworkRequest request;
        request.setUrl(QUrl::fromEncoded(urlString.toLatin1()));
        request.setRawHeader("accept", "*/*");
        request.setRawHeader("user-agent", UserAgent);
        label->setText(tr("Preparing"));
        httpObject->get(request);
        reply->deleteLater();
    } else {
        QString content = QString::fromUtf8(reply->readAll().constData());
        QString captured{};
        const QRegularExpression dataRegex(LyricsProviders::providers[providerNum].dataRegex,
                                           QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatchIterator matchIter = dataRegex.globalMatch(content);
        if(!matchIter.hasNext()) {
            lyricsBrowser->setHtml(tr("No lyrics available"));
            reply->deleteLater();
            if(cycleThroughProviders) {
                checkNextProvider();
            }
            return;
        } else {
            QRegularExpressionMatch match = matchIter.next();
            captured.append(match.captured(1).trimmed());
        }
        for(const auto& [find, replace] : provider.dataReplace) {
            captured = captured.replace(find, replace);
        }
        for(const auto& exclude : provider.dataExcludeRegex) {
            captured = captured.replace(
                QRegularExpression(exclude,
                                   QRegularExpression::DotMatchesEverythingOption), QString());
        }
        captured.append(QString(QSL("\n<a href=\"%1\">%1</a>")).arg(
                            reply->url().toString()));
        lyricsBrowser->setHtml(
                    captured
                    .replace(QChar::fromLatin1('\n'), QLatin1String("<br />")));
        reply->deleteLater();
    }
}

QString LyricsDialog::replace(QString string) {
    const LyricsProviders::Provider& provider = LyricsProviders::providers[providerNum];
    for(const auto& [rep, chr] : provider.searchReplace) {
        for(const QChar& c : rep) {
            string = string.replace(c, chr);
        }
    }
    return string;
}

QString LyricsDialog::escapeRegexInput(QString string) {
    const QString rep { QStringLiteral("^$()<>[{\\|*+?") };
    for(const QChar& c: rep) {
        string = string.replace(c, QString::fromLatin1("\\") + c);
    }
    return string;
}

void LyricsDialog::checkNextProvider() {
    providerNum++;
    if(providerNum < LyricsProviders::providers.size()) {
        search();
    }
}

void LyricsDialog::update() {
    if(LyricsProviders::providers.isEmpty()) {
        return;
    }
    // check if provider number is out of bounds
    if(providerNum < 0 || providerNum >= LyricsProviders::providers.size()) {
        providerNum = 0;
        cycleThroughProviders = true;
    }
    artistLineEdit->setText(PLAYER->getTrack().artist);
    titleLineEdit->setText(PLAYER->getTrack().title);
    if(!artistLineEdit->text().isEmpty()) {
        search();
    }
}

void LyricsDialog::search() {
    label->setText(tr("Fetching"));
    lyricsBrowser->clear();
    setWindowTitle(QString(QStringLiteral("%1 - %2"))
                   .arg(artistLineEdit->text(), titleLineEdit->text()));
    QNetworkRequest request;
    QString artist = artistLineEdit->text();
    QString title = titleLineEdit->text();
    const QChar firstLetterArtist = artistLineEdit->text().at(0).toLower();
    const LyricsProviders::Provider& provider = LyricsProviders::providers[providerNum];
    if(provider.urlTemplate.isEmpty()) {
        artist = artist.toLower();
        title = title.toLower();
    }
    request.setUrl(
        QUrl((provider.searchUrl.contains(QSL("%3"))
                  ? provider.searchUrl.arg(replace(artist), replace(title), firstLetterArtist)
                  : provider.searchUrl.arg(replace(artist), replace(title)))
                 .replace(QRegularExpression(QSL("-{2,}")), QChar::fromLatin1('-'))));
    request.setRawHeader("accept", "*/*");
    request.setRawHeader("user-agent", UserAgent);
    if(!provider.urlTemplate.isEmpty()) {
        replyObject = httpObject->get(request);
    } else {
        replyObject = nullptr;
        httpObject->get(request);
    }
}
