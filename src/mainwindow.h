#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QTimer>
#include <msdp2xxx.h>

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
    void on_powerCheckBox_toggled(bool checked);
    void on_measurePushButton_clicked();
    void on_reverseCheckBox_toggled(bool checked);

private:
    Ui::MainWindow *ui;
    static const char pol_pm[];
    static const char pol_mp[];
    sdp_t sdp;
    QSettings settings;
    QTimer currentTimer;
    double currentOld;

    bool openDevs();
    void closeDevs();
    void updateCurrent();
};

#endif // MAINWINDOW_H
