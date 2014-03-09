#ifndef LYRICSDIALOG_H
#define LYRICSDIALOG_H
#if QT_VERSION >= 0x050000
    #include <QtWidgets>
#endif

#include <QWidget>

#include "ui_lyricsdialog.h"

class QAction;
class QLabel;
class QNetworkAccessManager;
class QNetworkReply;
class PlayerInterface;

class LyricsDialog : public QWidget
{
    Q_OBJECT

    PlayerInterface *m_player;

    Ui::LyricsDialog ui;
    QNetworkAccessManager *m_http;
    QNetworkReply *m_requestReply;
    QString m_artist, m_title;
    QString getArtist();
    QString getTitle();
    void search();
    QString format(QString);

public:
    LyricsDialog(QWidget *parent = 0, PlayerInterface *player = 0);

private slots:
    void showText(QNetworkReply *reply);
    void on_artistLineEdit_returnPressed();
    void on_titleLineEdit_returnPressed();
    void on_updatePushButton_clicked();
};

#endif // LYRICSDIALOG_H
