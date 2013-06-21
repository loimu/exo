#ifndef EXO_H
#define EXO_H

#if QT_VERSION >= 0x050000
    #include <QtWidgets>
#endif
#include <QSystemTrayIcon>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QAction;
//class QCheckBox;
//class QComboBox;
//class QGroupBox;
//class QLabel;
//class QLineEdit;
class QMenu;
//class QPushButton;
//class QSpinBox;
//class QTextEdit;
QT_END_NAMESPACE

class Exo : public QWidget
{
    Q_OBJECT

public:
    Exo();

signals:

private slots:
    void clicked(QSystemTrayIcon::ActivationReason);
    void updateToolTip();
    void play();
    void pause();
    void prev();
    void next();
    void stop();
    void quit();
    void openWindow();
    void showLyricsWindow();
    void updateInfo();

private:
    void createActions();
    void createTrayIcon();
    bool serverRunning();
    void runServer();
    QString coverPath();

    QStringList m_info;

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

#endif // EXO_H
