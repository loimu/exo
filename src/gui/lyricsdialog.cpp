/* ========================================================================
*    Copyright (C) 2013-2021 Blaze <blaze@vivaldi.net>
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
#include <QAction>

#include "playerinterface.h"
#include "lyricsdialog.h"

#define QSL QStringLiteral


struct Provider {
    const QString searchUrl;
    const QString urlTemplate;
    const QString urlRegExp;
    const QString dataRegExp;
};

const QVector<Provider> LyricsDialog::providers = {
    { QSL("https://www.musixmatch.com/search/%1 %2/tracks"),
      QSL("https://www.musixmatch.com%1"),
      QSL("<a class=\"title\" href=\"([^\"]*)"),
      QSL("\\<p class=\"mxm-lyrics__content \">(.*?)\\</p\\>")},
    { QSL("https://www.metal-archives.com/search/ajax-advanced/searching/songs/"
      "?songTitle=%2&amp;bandName=%1&amp;ExactBandMatch=1"),
      QSL("https://www.metal-archives.com/release/ajax-view-lyrics/id/%1"),
      QSL("lyricsLink_(\\d+)"), QSL("(.*)")}
};

LyricsDialog::LyricsDialog(QWidget* parent) : BaseDialog(parent),
    httpObject(new QNetworkAccessManager(this)),
    replyObject(nullptr)
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
    auto* spacer = new QSpacerItem(0, 0,
                                   QSizePolicy::Expanding, QSizePolicy::Fixed);
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
    auto* switchNext = new QAction(this);
    switchNext->setShortcut(Qt::CTRL + Qt::Key_N);
    connect(switchNext, &QAction::triggered, this, [this] { index = 1; });
    this->addAction(switchNext);
    auto* switchPrev = new QAction(this);
    switchPrev->setShortcut(Qt::CTRL + Qt::Key_P);
    connect(switchPrev, &QAction::triggered, this, [this] { index = 0; });
    this->addAction(switchPrev);
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
        if(artistLineEdit->text() != format(PLAYER->getTrack().artist)
                || titleLineEdit->text() != format(
                    PLAYER->getTrack().title)) update();
    });
    update();
}

void LyricsDialog::showText(QNetworkReply* reply) {
    if(reply->error() != QNetworkReply::NoError) {
        label->setText(tr("Network error"));
        lyricsBrowser->setText(reply->errorString());
        replyObject = nullptr;
        reply->deleteLater();
        return;
    } else {
        label->setText(QStringLiteral("OK"));
    }

    if(replyObject == reply) {
        replyObject = nullptr;
        QRegularExpression re(LyricsDialog::providers.at(index).urlRegExp);
        QString content = QString::fromUtf8(reply->readAll().constData());
        reply->deleteLater();
        QRegularExpressionMatch match = re.match(content);
        if(!match.hasMatch()) {
            lyricsBrowser->setHtml(QSL("<b>") + tr("Not found") + QSL("</b>"));
            return;
        }
        QString urlString = LyricsDialog::providers.at(index).urlTemplate
                .arg(match.captured(1));
        QNetworkRequest request;
        request.setUrl(QUrl::fromEncoded(urlString.toLatin1()));
        request.setRawHeader("accept", "*/*");
        request.setRawHeader("user-agent", "Mozilla/5.0");
        label->setText(tr("Preparing"));
        httpObject->get(request);
        reply->deleteLater();
    } else {
        QString content = QString::fromUtf8(reply->readAll().constData());
        QRegularExpression re(LyricsDialog::providers.at(index).dataRegExp,
                    QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch match = re.match(content);
        QString captured{};
        QRegularExpressionMatchIterator matchIter = re.globalMatch(content);
        if(!matchIter.hasNext())
            captured.append(tr("No lyrics available"));
        while(matchIter.hasNext()) {
            QRegularExpressionMatch match = matchIter.next();
            captured.append(match.captured(1).trimmed());
            captured.append("<br />");
        }
        captured.append(QString(QSL("<p><a href=\"%1\">%1</a></p>"))
                        .arg(reply->url().toString()));
        lyricsBrowser->setHtml(
                    captured
                    .replace(QChar::fromLatin1('\n'), QLatin1String("<br />")));
        reply->deleteLater();
    }
}

QString LyricsDialog::format(QString string) {
    return string.replace(QChar::fromLatin1('&'), QLatin1String("and"));
}

QString LyricsDialog::replace(QString string) {
    const QString rep { QStringLiteral("_@,;&\\/\"") };
    for(const QChar& c: rep)
        string = string.replace(c, QChar::fromLatin1('-'));
    return string;
}

void LyricsDialog::update() {
    artistLineEdit->setText(format(PLAYER->getTrack().artist));
    titleLineEdit->setText(format(PLAYER->getTrack().title));
    if(!artistLineEdit->text().isEmpty())
        search();
}

void LyricsDialog::search() {
    label->setText(tr("Fetching"));
    lyricsBrowser->clear();
    setWindowTitle(QString(QStringLiteral("%1 - %2"))
                   .arg(artistLineEdit->text(), titleLineEdit->text()));
    QNetworkRequest request;
    request.setUrl(QUrl(QString(LyricsDialog::providers.at(index).searchUrl)
                        .arg(replace(artistLineEdit->text()),
                             replace(titleLineEdit->text()))));
    request.setRawHeader("accept", "*/*");
    request.setRawHeader("user-agent", "Mozilla/5.0");
    replyObject = httpObject->get(request);
}
