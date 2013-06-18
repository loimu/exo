#ifndef LYRICSWINDOW_H
#define LYRICSWINDOW_H

#include <QWidget>
#include <QApplication>
#include "ui_lyricswindow.h"

class QAction;
class QLabel;
class FindDialog;
class Spreadsheet;
class QNetworkAccessManager;
class QNetworkReply;

class LyricsWindow : public QWidget
{
    Q_OBJECT
public:
    LyricsWindow(const QString &artist, const QString &title, QWidget *parent = 0);
    ~LyricsWindow();

private slots:
    void showText(QNetworkReply *reply);
    void on_searchPushButton_clicked();

private:
    Ui::LyricsWindow ui;
    QNetworkAccessManager *m_http;
    QNetworkReply *m_requestReply;
    QString m_artist, m_title;
};

#endif // LYRICSWINDOW_H
