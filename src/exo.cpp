#include "exo.h"
#include "trayicon.h"

Exo::Exo(int &argc, char **argv) : QApplication(argc, argv)
{
    TrayIcon *trayicon = new TrayIcon();
    trayicon->hide();
}
