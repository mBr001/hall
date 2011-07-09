#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QTimer>
#include "configui.h"
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

public slots:
    void closeEvent(QCloseEvent *event);
    void show();

private slots:
    void on_closePushButton_clicked();
    void on_currentDoubleSpinBox_valueChanged(double value);
    void on_currentTimer_timeout();
    void on_measurePushButton_clicked();
    void on_powerCheckBox_toggled(bool checked);
    void on_reverseCheckBox_toggled(bool checked);

private:
    Ui::MainWindow *ui;
    /** HTML string to show colored "+ -". */
    static const char pol_pm[];
    /** HTML string to show colored "- +". */
    static const char pol_mp[];
    /** Mansons SDP power supply driver. */
    sdp_t sdp;
    QSettings settings;
    /** Timer used to adjust current trought magnet in specified time. */
    QTimer currentTimer;
    /** Slope of current change flowing trought magnet [A/sec]. */
    static const float currentSlope = 0.001;
    /** Maximal value of current posilbe drain from power source. */
    float currentMax;
    /** Configuration dialog. */
    ConfigUI *configUI;

    /** Close all devices, eg. power supply, Agilent, switch, ... */
    void closeDevs();
    /** Show configration dialog and open and configure devices.

    @return true when user wants to measure and devices are ready,
        false otherwise. */
    bool getConfig();
    /** Open all devices. */
    bool openDevs();
    /** One step of current change towards wanted value. */
    void updateCurrent();
};

#endif // MAINWINDOW_H
