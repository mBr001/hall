#include <QtGui/QApplication>
#include <QMessageBox>
#include "error.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForTr(codec);

    a.setApplicationName("Hall");
    a.setOrganizationDomain("www.vscht.cz");
    a.setOrganizationName("VŠCHT");

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
