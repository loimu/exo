#include <cstdlib>

#include <QtGui>
#include <QApplication>

#include "exo.h"

Exo::Exo() {

    createActions();
    createTrayIcon();

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
}

int Exo::play() {
    int ret = std::system("mocp -p");
    return ret;
}

int Exo::pause() {
    int ret = std::system("mocp -G");
    return ret;
}

int Exo::prev() {
    int ret = std::system("mocp -r");
    return ret;
}

int Exo::next() {
    int ret = std::system("mocp -f");
    return ret;
}

int Exo::stop() {
    int ret = std::system("mocp -s");
    return ret;
}

int Exo::quit() {
    //int ret = std::system("mocp -x");
    return 0;
}
