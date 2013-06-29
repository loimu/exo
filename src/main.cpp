#include <QtGui>
#include "exo.h"

int main(int argc, char *argv[]) {

    Q_INIT_RESOURCE(exo);

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(0, QObject::tr("Systray"),
                              QObject::tr("I couldn't detect any system tray "
                                          "on this system."));
        return 1;
    }
    QApplication::setQuitOnLastWindowClosed(false);

    Exo app(argc, argv);
    return app.exec();
}
