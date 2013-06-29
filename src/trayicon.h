#ifndef TRAYICON_H
#define TRAYICON_H
#if QT_VERSION >= 0x050000
    #include <QtWidgets>
#endif

#include <QWidget>
#include <QSystemTrayIcon>

class PlayerInterface;

class TrayIcon : public QWidget
{
    Q_OBJECT
public:
    explicit TrayIcon(QWidget *parent = 0);
    
signals:

private slots:
    void clicked(QSystemTrayIcon::ActivationReason);
    void updateToolTip();
    void showLyricsWindow();
    
public slots:

private:
    void createActions();
    void createTrayIcon();
    QString coverPath();

    PlayerInterface *m_player;

    QMenu *trayIconMenu;

    QAction *lyricsAction;
    QAction *playAction;
    QAction *pauseAction;
    QAction *prevAction;
    QAction *nextAction;
    QAction *stopAction;
    QAction *quitAction;

    QSystemTrayIcon *trayIcon;
    
};

#endif // TRAYICON_H
