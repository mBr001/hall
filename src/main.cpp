#include <QtGui/QApplication>
#include "mainwindow.h"
#include "configui.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ConfigUI *configUI = new ConfigUI();

    if (configUI->exec() != QDialog::Accepted)
        return 0;
    delete configUI;

    MainWindow w;
    w.show();

    return a.exec();
}
