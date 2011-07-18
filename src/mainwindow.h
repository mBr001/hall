#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFile>
#include <QMainWindow>
#include <QSettings>
#include <QTimer>
#include <msdp2xxx.h>

#include "configui.h"
#include "powpolswitch.h"
#include "ps6220hack.h"
#include "hp34970hack.h"


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void startApp();

public slots:
    void closeEvent(QCloseEvent *event);
    void show();

private slots:
    void on_automationTimer_timeout();
    void on_coilCurrDoubleSpinBox_valueChanged(double );
    void on_coilPolCrossCheckBox_toggled(bool checked);
    void on_coilPowerCheckBox_toggled(bool checked);
    void on_currentTimer_timeout();
    void on_measurePushButton_clicked();
    void on_sampleCurrDoubleSpinBox_valueChanged(double );
    void on_samplePolCrossCheckBox_toggled(bool checked);
    void on_samplePowerCheckBox_toggled(bool checked);
    void on_startPushButton_clicked();

private:
    typedef struct {
        /** Function to call.
            @return true if next function should be called, false should cause
                automation process abort. */
        bool (*func)(MainWindow *this_);
        /** Delay before next function execution */
        int delay;
    } automationStep_t;

    /** Fully automated measurement in progress */
    bool autoRunning;
    /** Array of steps and loop marks for automated Hall measurement. */
    static const automationStep_t autoSteps[];
    /** Vector of steps created from autoSteps */
    static const QVector<automationStep_t> autoStepsVect;
    /** Current step of automated Hall measurement. */
    std::vector<automationStep_t>::const_iterator autoStepCurrent;
    /** Dinamic "mark" in hall automation steps for loops. */
    std::vector<automationStep_t>::const_iterator autoStepMark;
    /** Timer used for fully automated testing process. */
    QTimer automationTimer;
    /** Configuration dialog. */
    ConfigUI configUI;
    /** HTML string to show colored "+ -". */
    static const char pol_pn[];
    /** HTML string to show colored "- +". */
    static const char pol_np[];
    /** Mansons SDP power supply driver. */
    sdp_t sdp;
    /** Application settings */
    QSettings settings;
    /** Timer used to adjust current trought magnet in specified time. */
    QTimer coilTimer;
    /** Delay betwen current value update [ms].
     * BAD CODE INSIDE, DO NOT CHANGE! */
    static const int currentDwell = 1000;
    /** Slope of current change flowing trought magnet [A/sec]. */
    static const float currentSlope = 0.01;
    /** Power source output current limit. */
    float currentMax;
    /** Power polarity switch handler. */
    PwrPolSwitch pwrPolSwitch;
    /** Keithlay PS 6220 hacky class. */
    PS6220Hack ps622Hack;
    /** File to same measured data. */
    QFile csvFile;
    /** HP 34970A device to measure voltage and resistivity. */
    HP34970hack hp34970Hack;
    /** User interface widgets */
    Ui::MainWindow *ui;

    /* Steps for Hall measurement automation */
    static bool auto00(MainWindow *this_);
    static bool auto01(MainWindow *this_);
    static bool auto02(MainWindow *this_);
    static bool auto03(MainWindow *this_);
    static bool auto04(MainWindow *this_);
    static bool auto05(MainWindow *this_);
    static bool auto06(MainWindow *this_);
    static bool auto07(MainWindow *this_);
    static bool auto08(MainWindow *this_);
    static bool auto09(MainWindow *this_);
    static bool auto10(MainWindow *this_);
    static bool autoMark(MainWindow *this_);
    static bool autoStop(MainWindow *this_);
    /** Close all devices, eg. power supply, Agilent, switch, ... */
    void closeDevs();
    /** Open all devices. */
    bool openDevs();
};

#endif // MAINWINDOW_H
