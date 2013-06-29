#include <QProcess>
#include <QTimer>

#include "playerinterface.h"

PlayerInterface *PlayerInterface::m_instance=0;

PlayerInterface::PlayerInterface(QObject *parent) :
    QObject(parent) {
    if(m_instance)
        qFatal("PlayerInterface: only one instance is allowed");
    m_instance = this;

    if(!isServerRunning())
        runServer();

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateInfo()));
    connect(timer, SIGNAL(timeout()), parent, SLOT(updateToolTip()));
    timer->start(1000);
}

PlayerInterface::~PlayerInterface() {
    m_instance = 0;
}

bool PlayerInterface::isServerRunning() {
    QProcess proc;
    proc.start("pidof", QStringList() << "mocp");
    proc.waitForFinished(-1);
    QString output = QString::fromUtf8(proc.readAllStandardOutput());
    if(output.length() > 1)
        return true;
    else
        return false;
}

void PlayerInterface::runServer() {
    QProcess proc;
    proc.startDetached("mocp", QStringList() << "-S");
}

void PlayerInterface::play() {
    QProcess proc;
    proc.startDetached("mocp", QStringList() << "-p");
}

void PlayerInterface::pause() {
    QProcess proc;
    proc.startDetached("mocp", QStringList() << "-G");
}

void PlayerInterface::prev() {
    QProcess proc;
    proc.startDetached("mocp", QStringList() << "-r");
}

void PlayerInterface::next() {
    QProcess proc;
    proc.startDetached("mocp", QStringList() << "-f");
}

void PlayerInterface::stop() {
    QProcess proc;
    proc.startDetached("mocp", QStringList() << "-s");
}

void PlayerInterface::quit() {
    QProcess proc;
    proc.startDetached("mocp", QStringList() << "-x");
}

void PlayerInterface::updateInfo() {
    QProcess proc;
    proc.start("mocp", QStringList() << "-i");
    proc.waitForFinished(-1);
    QString output = QString::fromUtf8(proc.readAllStandardOutput());
    QStringList list = output.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
    list.replaceInStrings(QRegExp("(\\w+:\\s)+(.*)"), "\\2");
//    for (int i = 0; i < list.size(); ++i) {
//        qDebug("debug: %s", qPrintable(list.at(i)));
//    }
    m_list = list;
}

void PlayerInterface::openWindow() {
    QProcess proc;
    proc.startDetached("x-terminal-emulator", QStringList() << "-e" << "mocp");
}
