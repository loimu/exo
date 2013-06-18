#include <QtGui>
#include <QTimer>
#include <QProcess>

#include <QDebug>

#include "exo.h"
#include "lyricswindow.h"

Exo::Exo() {

    createActions();
    createTrayIcon();
    if(!serverRunning())
        runServer();

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateToolTip()));
    timer->start(5000);

    trayIcon->show();
}

void Exo::createActions() {
    lyricsAction = new QAction(tr("&Lyrics"),this);
    connect(lyricsAction, SIGNAL(triggered()), this, SLOT(showLyricsWindow()));
    playAction = new QAction(tr("&Play"),this);
    connect(playAction, SIGNAL(triggered()), this, SLOT(play()));
    QIcon playIcon(":/images/play.png");
    playAction->setIcon(playIcon);
    pauseAction = new QAction(tr("P&ause"),this);
    connect(pauseAction, SIGNAL(triggered()), this, SLOT(pause()));
    QIcon pauseIcon(":/images/pause.png");
    pauseAction->setIcon(pauseIcon);
    prevAction = new QAction(tr("P&rev"),this);
    connect(prevAction, SIGNAL(triggered()), this, SLOT(prev()));
    QIcon prevIcon(":/images/prev.png");
    prevAction->setIcon(prevIcon);
    nextAction = new QAction(tr("&Next"),this);
    connect(nextAction, SIGNAL(triggered()), this, SLOT(next()));
    QIcon nextIcon(":/images/next.png");
    nextAction->setIcon(nextIcon);
    stopAction = new QAction(tr("&Stop"),this);
    connect(stopAction, SIGNAL(triggered()), this, SLOT(stop()));
    QIcon stopIcon(":/images/stop.png");
    stopAction->setIcon(stopIcon);
    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), this, SLOT(quit()));
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    QIcon quitIcon(":/images/close.png");
    quitAction->setIcon(quitIcon);
}

void Exo::createTrayIcon() {
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(lyricsAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(playAction);
    trayIconMenu->addAction(pauseAction);
    trayIconMenu->addAction(prevAction);
    trayIconMenu->addAction(nextAction);
    trayIconMenu->addAction(stopAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    QIcon icon(":/images/exo.png");
    trayIcon->setIcon(icon);

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                      SLOT(clicked(QSystemTrayIcon::ActivationReason)));
}

bool Exo::serverRunning() {
    QProcess proc;
    proc.start("pidof", QStringList() << "mocp");
    proc.waitForFinished(-1);
    QString output = QString::fromUtf8(proc.readAllStandardOutput());
    if(output.length() > 1)
        return true;
    else
        return false;
}

void Exo::runServer() {
    QProcess proc;
    proc.startDetached("mocp", QStringList() << "-S");
}

QString Exo::getArtist() {
    QProcess proc;
    proc.start("mocp", QStringList() << "-Q" << "%artist");
    proc.waitForFinished(-1);
    QString output = QString::fromUtf8(proc.readAllStandardOutput());
    return output.simplified();
}

QString Exo::getTitle() {
    QProcess proc;
    proc.start("mocp", QStringList() << "-Q" << "%song");
    proc.waitForFinished(-1);
    QString output = QString::fromUtf8(proc.readAllStandardOutput());
    return output.simplified();
}

void Exo::clicked(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
        case QSystemTrayIcon::DoubleClick:
            openWindow();
            break;
        case QSystemTrayIcon::Trigger:
            break;
        case QSystemTrayIcon::MiddleClick:
            pause();
            break;
        default:
            break;
    }
}

void Exo::updateToolTip() {
    QProcess proc;
    proc.start("mocp", QStringList() << "-Q" << "%title");
    proc.waitForFinished(-1);
    QString output = QString::fromUtf8(proc.readAllStandardOutput());
    trayIcon->setToolTip(output.simplified());
}

void Exo::updateInfo() {
    QProcess proc;
    proc.start("mocp", QStringList() << "-i");
    proc.waitForFinished(-1);
    QString output = QString::fromUtf8(proc.readAllStandardOutput());
    QStringList list = output.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);
    for (int i = 0; i < list.size(); ++i)
        qDebug() << list.at(i);
}

void Exo::play() {
    QProcess proc;
    proc.startDetached("mocp", QStringList() << "-p");
}

void Exo::pause() {
    QProcess proc;
    proc.startDetached("mocp", QStringList() << "-G");
}

void Exo::prev() {
    QProcess proc;
    proc.startDetached("mocp", QStringList() << "-r");
}

void Exo::next() {
    QProcess proc;
    proc.startDetached("mocp", QStringList() << "-f");
}

void Exo::stop() {
    QProcess proc;
    proc.startDetached("mocp", QStringList() << "-s");
}

void Exo::quit() {
    QProcess proc;
    proc.startDetached("mocp", QStringList() << "-x");
}

void Exo::openWindow() {
    QProcess proc;
    proc.startDetached("x-terminal-emulator", QStringList() << "-e" << "mocp");
}

void Exo::showLyricsWindow() {
    QString artist = getArtist();
    QString title = getTitle();
    LyricsWindow *lyricsWindow = new LyricsWindow(artist, title);
    lyricsWindow->show();
}
