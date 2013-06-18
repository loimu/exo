#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QRegExp>

#include "lyricswindow.h"

LyricsWindow::LyricsWindow(const QString &artist, const QString &title, QWidget *parent)
        : QWidget(parent) {
    ui.setupUi(this);
    setWindowFlags(Qt::Dialog);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_QuitOnClose, false);
    m_requestReply = 0;
    m_http = new QNetworkAccessManager(this);
    connect(m_http, SIGNAL(finished (QNetworkReply *)), SLOT(showText(QNetworkReply *)));
    if(!artist.isEmpty()) {
        ui.artistLineEdit->setText(artist);
        ui.titleLineEdit->setText(title);
        on_searchPushButton_clicked();
    }
}

LyricsWindow::~LyricsWindow() {}

void LyricsWindow::showText(QNetworkReply *reply) {
    ui.stateLabel->setText(tr("Done"));
    if (reply->error() != QNetworkReply::NoError)
    {
        ui.stateLabel->setText(tr("Error"));
        ui.textBrowser->setText(reply->errorString());
        m_requestReply = 0;
        reply->deleteLater();
        return;
    }
    QString content = QString::fromUtf8(reply->readAll().constData());
    //qDebug("=%s=",qPrintable(content));

    if(m_requestReply == reply)
    {
        m_requestReply = 0;
        reply->deleteLater();
        QRegExp artist_regexp("<artist>(.*)</artist>");
        artist_regexp.setMinimal(true);
        QRegExp song_regexp("<song>(.*)</song>");
        song_regexp.setMinimal(true);
        QRegExp lyrics_regexp("<lyrics>(.*)</lyrics>");
        lyrics_regexp.setMinimal(true);
        QRegExp url_regexp("<url>(.*)</url>");
        url_regexp.setMinimal(true);

        if(artist_regexp.indexIn(content) < 0 || song_regexp.indexIn(content) < 0 || url_regexp.indexIn(content) < 0)
        {
            ui.textBrowser->setHtml("<b>" + tr("Error") + "</b>");
            return;
        }
        else {
            m_artist = artist_regexp.cap(1);
            m_title = song_regexp.cap(1);
        }

        if(lyrics_regexp.indexIn(content) < 0)
        {
            ui.textBrowser->setHtml("<b>" + tr("Error") + "</b>");
            return;
        }
        else if(lyrics_regexp.cap(1) == "Not found")
        {
            ui.textBrowser->setHtml("<b>" + tr("Not found") + "</b>");
            return;
        }

        QString temp = url_regexp.cap(1).toAscii();
        qDebug("LyricsWindow: received url = %s", qPrintable(temp));
        temp.replace("http://lyrics.wikia.com/","http://lyrics.wikia.com/index.php?title=");
        temp.append("&action=edit");

        QUrl url = QUrl::fromEncoded(temp.toAscii());
        QString referer = url_regexp.cap(1);
        qDebug("LyricsWindow: request url = %s", url.toEncoded().constData());
        QNetworkRequest request;
        request.setUrl(url);
        request.setRawHeader("Referer", referer.toAscii());
        ui.stateLabel->setText(tr("Receiving"));
        m_http->get(request);
        reply->deleteLater();
        return;
    }
    content.replace("&lt;", "<");
    QRegExp lyrics_regexp("<lyrics>(.*)</lyrics>");
    lyrics_regexp.indexIn(content);

    QString text = "<h2>" + m_artist + " - " + m_title + "</h2>";
    QString lyrics = lyrics_regexp.cap(1);
    lyrics = lyrics.trimmed();
    lyrics.replace("\n", "<br>");
    if(lyrics.isEmpty())
        ui.textBrowser->setHtml("<b>" + tr("Not found") + "</b>");
    else
    {
        text += lyrics;
        ui.textBrowser->setHtml(text);
    }
    reply->deleteLater();
}

void LyricsWindow::on_searchPushButton_clicked() {
    ui.stateLabel->setText(tr("Receiving"));
    setWindowTitle(QString(tr("Lyrics: %1 - %2")).arg(ui.artistLineEdit->text())
                   .arg(ui.titleLineEdit->text()));
    QNetworkRequest request;
    request.setUrl(QUrl("http://lyrics.wikia.com/api.php?action=lyrics&artist=" +
                        ui.artistLineEdit->text()+"&song=" + ui.titleLineEdit->text() + "&fmt=xml"));
    request.setRawHeader("User-Agent", QString("Mozilla/5.0").toAscii());
    m_requestReply = m_http->get(request);
}
