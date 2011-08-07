#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Hall");
    a.setOrganizationDomain("www.vscht.cz");
    a.setOrganizationName(QString::fromLocal8Bit("VŠCHT"));

    MainWindow w;
    w.startApp();

    return a.exec();
}
