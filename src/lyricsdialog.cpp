/* ========================================================================
*    Copyright (C) 2013-2015 Blaze <blaze@open.by>
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
#include <QKeyEvent>

#include "playerinterface.h"
#include "lyricsdialog.h"

LyricsDialog::LyricsDialog(QWidget *parent) : QWidget(parent)
{
    ui.setupUi(this);
    setWindowFlags(Qt::Dialog);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_QuitOnClose, false);
    replyObject = 0;
    httpObject = new QNetworkAccessManager(this);
    connect(httpObject, SIGNAL(finished(QNetworkReply *)),
            SLOT(showText(QNetworkReply *)));
    on_updatePushButton_released();
}

void LyricsDialog::showText(QNetworkReply *reply) {
    ui.stateLabel->setText("OK");
    if(reply->error() != QNetworkReply::NoError) {
        ui.stateLabel->setText(tr("Network error"));
        ui.textBrowser->setText(reply->errorString());
        replyObject = 0;
        reply->deleteLater();
        return;
    }
    QString content = QString::fromUtf8(reply->readAll().constData());
    if(replyObject == reply) {
        replyObject = 0;
        reply->deleteLater();
        QRegExp songRgx("<artist>(.*)</artist>.*<song>(.*)</song>.*"
                        "<lyrics>(.*)</lyrics>.*<url>(.*)</url>");
        songRgx.setMinimal(true);
        if(songRgx.indexIn(content) < 0) {
            ui.textBrowser->setHtml("<b>" + tr("Error") + "</b>");
            return;
        }
        else if(songRgx.cap(3) == "Not found") {
            ui.textBrowser->setHtml("<b>" + tr("Not found") + "</b>");
            return;
        }
        else {
            artistString = songRgx.cap(1);
            titleString = songRgx.cap(2);
        }
        QString urlString = songRgx.cap(4).toLatin1();
        urlString.replace("http://lyrics.wikia.com/",
                     "http://lyrics.wikia.com/index.php?title=");
        urlString.append("&action=edit");
        QNetworkRequest request;
        request.setUrl(QUrl::fromEncoded(urlString.toLatin1()));
        request.setRawHeader("Referer", songRgx.cap(4).toLatin1());
        ui.stateLabel->setText(tr("Downloading"));
        httpObject->get(request);
        reply->deleteLater();
        return;
    }
    QRegExp lyricsRgx("&lt;lyrics>(.*)&lt;/lyrics>");
    lyricsRgx.indexIn(content);
    QString text = QString("<h2>%1 - %2</h2>").arg(artistString).arg(titleString);
    QString lyrics = lyricsRgx.cap(1);
    lyrics = lyrics.trimmed();
    lyrics.replace("\n", "<br>");
    if(lyrics.isEmpty())
        text += "<b>" + tr("There's no lyrics for some reason.") + "</b>";
    else
        text += lyrics;
    ui.textBrowser->setHtml(text);
    reply->deleteLater();
}

void LyricsDialog::on_artistLineEdit_returnPressed() {
    search();
}

void LyricsDialog::on_titleLineEdit_returnPressed() {
    search();
}

void LyricsDialog::on_updatePushButton_released() {
    PlayerInterface* player = PlayerInterface::instance();
    ui.artistLineEdit->setText(format(player->trackObject()->artist));
    ui.titleLineEdit->setText(format(player->trackObject()->song));
    if(!ui.artistLineEdit->text().isEmpty())
        search();
}

void LyricsDialog::on_prevButton_released() {
    PlayerInterface::instance()->prev();
    ui.textBrowser->setHtml(tr("Please wait a second"));
    QTimer::singleShot(1500, this, SLOT(on_updatePushButton_released()));
}

void LyricsDialog::on_nextButton_released() {
    PlayerInterface::instance()->next();
    ui.textBrowser->setHtml(tr("Please wait a second"));
    QTimer::singleShot(1500, this, SLOT(on_updatePushButton_released()));
}

QString LyricsDialog::format(QString string) {
    string.replace("&", "and");
    return string;
}

void LyricsDialog::search() {
    ui.stateLabel->setText(tr("Searching"));
    setWindowTitle(QString(tr("%1 - %2")).arg(ui.artistLineEdit->text())
                   .arg(ui.titleLineEdit->text()));
    QNetworkRequest request;
    request.setUrl(QUrl("http://lyrics.wikia.com/api.php?action=lyrics&artist="+
                        ui.artistLineEdit->text() + "&song=" +
                        ui.titleLineEdit->text() + "&fmt=xml"));
    request.setRawHeader("User-Agent", QString("Mozilla/5.0").toLatin1());
    replyObject = httpObject->get(request);
}

void LyricsDialog::keyPressEvent(QKeyEvent *e) {
    if(e->key() == Qt::Key_Escape)
        this->close();
}
