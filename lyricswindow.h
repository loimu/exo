#ifndef LYRICSWINDOW_H
#define LYRICSWINDOW_H

#include <QWidget>
#include <QApplication>

#include "ui_lyricswindow.h"

class QAction;
class QLabel;
//class FindDialog;
//class Spreadsheet;
class QNetworkAccessManager;
class QNetworkReply;

class LyricsWindow : public QWidget
{
    Q_OBJECT
public:
    LyricsWindow(QWidget *parent = 0);

private slots:
    void showText(QNetworkReply *reply);
    void on_artistLineEdit_returnPressed();
    void on_titleLineEdit_returnPressed();
    void on_updatePushButton_clicked();

private:
    Ui::LyricsWindow ui;
    QNetworkAccessManager *m_http;
    QNetworkReply *m_requestReply;
    QString m_artist, m_title;
    QString getArtist();
    QString getTitle();
    void search();
};

#endif // LYRICSWINDOW_H
