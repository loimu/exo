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

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QRegExp>
#include <QTimer>

#include "core/playerinterface.h"
#include "lyricsdialog.h"
#include "ui_lyricsdialog.h"

LyricsDialog::LyricsDialog(QWidget *parent) : BaseDialog(parent),
    ui(new Ui::LyricsDialog),
    httpObject(new QNetworkAccessManager(this)),
    replyObject(nullptr)
{
    ui->setupUi(this);
    connect(httpObject, SIGNAL(finished(QNetworkReply *)),
            SLOT(showText(QNetworkReply *)));
    on_updatePushButton_released();
}

void LyricsDialog::showText(QNetworkReply *reply) {
    ui->stateLabel->setText("OK");
    if(reply->error() != QNetworkReply::NoError) {
        ui->stateLabel->setText(tr("Network error"));
        ui->textBrowser->setText(reply->errorString());
        replyObject = nullptr;
        reply->deleteLater();
        return;
    }
    QString content = QString::fromUtf8(reply->readAll().constData());
    if(replyObject == reply) {
        replyObject = nullptr;
        reply->deleteLater();
        QRegExp songRgx(
                    QLatin1String("<artist>(.*)</artist>.*<song>(.*)</song>.*"
                                  "<lyrics>(.*)</lyrics>.*<url>(.*)</url>"));
        songRgx.setMinimal(true);
        if(songRgx.indexIn(content) < 0) {
            ui->textBrowser->setHtml(QLatin1String("<b>")
                                     + tr("Error")
                                     + QLatin1String("</b>"));
            return;
        }
        else if(songRgx.cap(3) == QLatin1String("Not found")) {
            ui->textBrowser->setHtml(QLatin1String("<b>")
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
        ui->stateLabel->setText(tr("Downloading"));
        httpObject->get(request);
        reply->deleteLater();
        return;
    }
    QRegExp lyricsRgx(QLatin1String("&lt;lyrics>(.*)&lt;/lyrics>"));
    lyricsRgx.indexIn(content);
    QString text = QString("<h2>%1 - %2</h2>")
            .arg(artistString)
            .arg(titleString);
    QString lyrics = lyricsRgx.cap(1);
    lyrics = lyrics.trimmed();
    lyrics.replace(QLatin1String("\n"), QLatin1String("<br>"));
    if(lyrics.isEmpty())
        text += QLatin1String("<b>")
                + tr("There're no lyrics for some reason.")
                + QLatin1String("</b>");
    else
        text += lyrics;
    ui->textBrowser->setHtml(text);
    reply->deleteLater();
}

void LyricsDialog::on_artistLineEdit_returnPressed() {
    search();
}

void LyricsDialog::on_titleLineEdit_returnPressed() {
    search();
}

void LyricsDialog::on_updatePushButton_released() {
    PlayerInterface* player = PlayerInterface::self();
    ui->artistLineEdit->setText(format(player->trackObject()->artist));
    ui->titleLineEdit->setText(format(player->trackObject()->song));
    if(!ui->artistLineEdit->text().isEmpty())
        search();
}

void LyricsDialog::on_prevButton_released() {
    PlayerInterface::self()->prev();
    ui->textBrowser->setHtml(tr("Please wait a second"));
    QTimer::singleShot(1500, this, SLOT(on_updatePushButton_released()));
}

void LyricsDialog::on_nextButton_released() {
    PlayerInterface::self()->next();
    ui->textBrowser->setHtml(tr("Please wait a second"));
    QTimer::singleShot(1500, this, SLOT(on_updatePushButton_released()));
}

QString LyricsDialog::format(QString string) {
    string.replace(QLatin1String("&"), QLatin1String("and"));
    return string;
}

void LyricsDialog::search() {
    ui->stateLabel->setText(tr("Searching"));
    setWindowTitle(QString("%1 - %2").arg(ui->artistLineEdit->text())
                   .arg(ui->titleLineEdit->text()));
    QNetworkRequest request;
    request.setUrl(
                QUrl(QLatin1String("http://lyrics.wikia.com/api.php"
                                   "?action=lyrics&artist=")
                     + ui->artistLineEdit->text() + QLatin1String("&song=")
                     + ui->titleLineEdit->text() + QLatin1String("&fmt=xml")));
    request.setRawHeader("User-Agent", "Mozilla/5.0");
    replyObject = httpObject->get(request);
}
