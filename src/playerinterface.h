#ifndef PLAYERINTERFACE_H
#define PLAYERINTERFACE_H

#include <QObject>
#include <QStringList>

class PlayerInterface : public QObject
{
    Q_OBJECT
public:
    PlayerInterface(QObject *parent = 0);
    ~PlayerInterface();

    bool isServerRunning();
    void openWindow();

    QStringList m_list;

signals:

public slots:
    void play();
    void pause();
    void prev();
    void next();
    void stop();
    void quit();

private slots:
    void updateInfo();

private:
    void runServer();

    static PlayerInterface* m_instance;

};

#endif // PLAYERINTERFACE_H
