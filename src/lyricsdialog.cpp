#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QRegExp>
#include <QTimer>

#include "playerinterface.h"
#include "lyricsdialog.h"

LyricsDialog::LyricsDialog(QWidget *parent) : QWidget(parent) {
    ui.setupUi(this);
    setWindowFlags(Qt::Dialog);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_QuitOnClose, false);
    m_requestReply = 0;
    m_http = new QNetworkAccessManager(this);
    connect(m_http, SIGNAL(finished(QNetworkReply *)),
            SLOT(showText(QNetworkReply *)));
    on_updatePushButton_clicked();
}

void LyricsDialog::showText(QNetworkReply *reply) {
    ui.stateLabel->setText(tr("Done"));
    if (reply->error() != QNetworkReply::NoError) {
        ui.stateLabel->setText(tr("Error"));
        ui.textBrowser->setText(reply->errorString());
        m_requestReply = 0;
        reply->deleteLater();
        return;
    }
    QString content = QString::fromUtf8(reply->readAll().constData());
    if(m_requestReply == reply) {
        m_requestReply = 0;
        reply->deleteLater();
        QRegExp artistRgx("<artist>(.*)</artist>");
        artistRgx.setMinimal(true);
        QRegExp songRgx("<song>(.*)</song>");
        songRgx.setMinimal(true);
        QRegExp lyricsRgx("<lyrics>(.*)</lyrics>");
        lyricsRgx.setMinimal(true);
        QRegExp urlRgx("<url>(.*)</url>");
        urlRgx.setMinimal(true);
        if(artistRgx.indexIn(content) < 0 || songRgx.indexIn(content) < 0 ||
           lyricsRgx.indexIn(content) < 0 || urlRgx.indexIn(content) < 0) {
            ui.textBrowser->setHtml("<b>" + tr("Error") + "</b>");
            return;
        }
        else if(lyricsRgx.cap(1) == "Not found") {
            ui.textBrowser->setHtml("<b>" + tr("Not found") + "</b>");
            return;
        }
        else {
            m_artist = artistRgx.cap(1);
            m_title = songRgx.cap(1);
        }
        QString urlString = urlRgx.cap(1).toLatin1();
        urlString.replace("http://lyrics.wikia.com/",
                     "http://lyrics.wikia.com/index.php?title=");
        urlString.append("&action=edit");
        QUrl url = QUrl::fromEncoded(urlString.toLatin1());
        QString referer = urlRgx.cap(1);
        QNetworkRequest request;
        request.setUrl(url);
        request.setRawHeader("Referer", referer.toLatin1());
        ui.stateLabel->setText(tr("Receiving"));
        m_http->get(request);
        reply->deleteLater();
        return;
    }
    content.replace("&lt;", "<");
    QRegExp lyricsRgx("<lyrics>(.*)</lyrics>");
    lyricsRgx.indexIn(content);
    QString text = "<h2>" + m_artist + " - " + m_title + "</h2>";
    QString lyrics = lyricsRgx.cap(1);
    lyrics = lyrics.trimmed();
    lyrics.replace("\n", "<br>");
    if(lyrics.isEmpty())
        ui.textBrowser->setHtml("<b>" + tr("Not found") + "</b>");
    else {
        text += lyrics;
        ui.textBrowser->setHtml(text);
    }
    reply->deleteLater();
}

void LyricsDialog::on_artistLineEdit_returnPressed() {
    search();
}

void LyricsDialog::on_titleLineEdit_returnPressed() {
    search();
}

void LyricsDialog::on_updatePushButton_clicked() {
    PlayerInterface* player = PlayerInterface::instance();
    ui.artistLineEdit->setText(format(player->artist()));
    ui.titleLineEdit->setText(format(player->title()));
    if(!player->artist().isEmpty())
        search();
}

void LyricsDialog::on_prevButton_clicked() {
    PlayerInterface::instance()->prev();
    ui.textBrowser->setHtml("");
    QTimer::singleShot(1500, this, SLOT(on_updatePushButton_clicked()));
}

void LyricsDialog::on_nextButton_clicked() {
    PlayerInterface::instance()->next();
    ui.textBrowser->setHtml("");
    QTimer::singleShot(1500, this, SLOT(on_updatePushButton_clicked()));
}

QString LyricsDialog::format(QString string) {
    string.replace("&", "and");
    return string;
}

void LyricsDialog::search() {
    ui.stateLabel->setText(tr("Receiving"));
    setWindowTitle(QString(tr("%1 - %2")).arg(ui.artistLineEdit->text())
                   .arg(ui.titleLineEdit->text()));
    QNetworkRequest request;
    request.setUrl(QUrl("http://lyrics.wikia.com/api.php?action=lyrics&artist="+
                        ui.artistLineEdit->text() + "&song=" +
                        ui.titleLineEdit->text() + "&fmt=xml"));
    request.setRawHeader("User-Agent", QString("Mozilla/5.0").toLatin1());
    m_requestReply = m_http->get(request);
}
