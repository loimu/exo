#include <QtGui>
#include <QTimer>
#include <QProcess>

#include "exo.h"

Exo::Exo() {

    createActions();
    createTrayIcon();

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateToolTip()));
    timer->start(10000);

    trayIcon->show();
}

//void Exo::closeEvent(QCloseEvent *event) {

//}

void Exo::createActions() {

    playAction = new QAction("P&lay",this);
    connect(playAction, SIGNAL(triggered()), this, SLOT(play()));
    QIcon playIcon(":/images/play.png");
    playAction->setIcon(playIcon);
    pauseAction = new QAction("&Pause",this);
    connect(pauseAction, SIGNAL(triggered()), this, SLOT(pause()));
    QIcon pauseIcon(":/images/pause.png");
    pauseAction->setIcon(pauseIcon);
    prevAction = new QAction("P&rev",this);
    connect(prevAction, SIGNAL(triggered()), this, SLOT(prev()));
    QIcon prevIcon(":/images/prev.png");
    prevAction->setIcon(prevIcon);
    nextAction = new QAction("&Next",this);
    connect(nextAction, SIGNAL(triggered()), this, SLOT(next()));
    QIcon nextIcon(":/images/next.png");
    nextAction->setIcon(nextIcon);
    stopAction = new QAction("&Stop",this);
    connect(stopAction, SIGNAL(triggered()), this, SLOT(stop()));
    QIcon stopIcon(":/images/stop.png");
    stopAction->setIcon(stopIcon);
    quitAction = new QAction("&Quit", this);
    connect(quitAction, SIGNAL(triggered()), this, SLOT(quit()));
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    QIcon quitIcon(":/images/close.png");
    quitAction->setIcon(quitIcon);
}

void Exo::createTrayIcon() {
    trayIconMenu = new QMenu(this);
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

void Exo::clicked(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
        case QSystemTrayIcon::DoubleClick:
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
    QString output = proc.readAllStandardOutput();
    trayIcon->setToolTip(output);
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
    //QProcess proc;
    //proc.startDetached("mocp", QStringList() << "-x");
}
