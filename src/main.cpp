#include <QtGui/QApplication>
#include <QMessageBox>
#include "error.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Hall");
    a.setOrganizationDomain("www.vscht.cz");
    a.setOrganizationName(QString::fromLocal8Bit("VŠCHT"));

    MainWindow w;
    w.startApp();

    try {
        return a.exec();
    }
    catch(Error &e)
    {
        QMessageBox::critical(0, e.description(), e.longDescription());
    }
    catch(std::runtime_error &e)
    {
        QMessageBox::critical(0, e.what(), e.what());
    }
}
