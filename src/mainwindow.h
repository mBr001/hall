#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_closePushButton_clicked();

private:
    Ui::MainWindow *ui;
    static const char pol_pm[];
    static const char pol_mp[];
};

#endif // MAINWINDOW_H
