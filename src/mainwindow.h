#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QTimer>
#include <msdp2xxx.h>

#include "configui.h"
#include "hp34970hack.h"
#include "powpolswitch.h"
#include "ps6220hack.h"
#include "qcsvfile.h"


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
    void on_measTimer_timeout();
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
    /** Comprises one timed task in measurement automation. */
    typedef struct {
        /** Function to call.
            @return true to continue measurement, false to abort measurement. */
        bool (*func)(MainWindow *this_);
        /** Delay before next step execution. */
        int delay;
    } Step_t;

    /** Series of measurement automation steps. */
    class Steps_t : public QVector<Step_t>
    {
    public:
        Steps_t() : QVector<Step_t>() {};
        /** Initiate steps from array of steps in form <begin, end).
            @par begin pointer to first element of array.
            @par end pointer behind last element of array.  */
        Steps_t(const Step_t *begin, const Step_t *end);
    };

    /** Channel offset for 34901A card */
    static const int _34901A;
    /** 34901A: sample pins cd */
    static const int _34901A_sample_cd;
    /** 34901A: sample pins da */
    static const int _34901A_sample_da;
    /** 34901A: sample pins bd */
    static const int _34901A_sample_bd;
    /** 34901A: sample pins ac */
    static const int _34901A_sample_ac;
    /** 34901A: hall probe */
    static const int _34901A_hall_probe;

    /** Channel offset for 34903A card */
    static const int _34903A;
    /** 34903A: sample pin a <-> current source (-) */
    static const int _34903A_sample_a_pwr_m;
    /** 34903A: sample pin b <-> current source (+) */
    static const int _34903A_sample_b_pwr_p;
    /** 34903A: sample pin c <-> current source (0/(+/-)) */
    static const int _34903A_sample_c_pwr_sw1;
    /** 34903A: sample pin d <-> current source (-) */
    static const int _34903A_sample_d_pwr_m;
    /** 34903A: current source (0/(+/-)) <-> current source (-) */
    static const int _34903A_pwr_sw1_pwr_m;
    /** 34903A: current source (0/(+/-)) <-> current source (+) */
    static const int _34903A_pwr_sw1_pwr_p;
    /** 34903A: hall probe - pin 1 <-> current source (-) */
    static const int _34903A_hall_probe_1_pwr_m;
    /** 34903A: hall probe - pin 2 <-> current source (+) */
    static const int _34903A_hall_probe_2_pwr_p;

    /** Indexes of columns in CSV file with data from experiment. */
    enum {
        csvColTime = 0,
        csvColHallProbeI,
        csvColHallProbeU,
        csvColSampleI,
        csvColSampleUacF,
        csvColSampleUacB,
        csvColSampleUbdF,
        csvColSampleUbdB,
        csvColSampleUcdF,
        csvColSampleUcdB,
        csvColSampleUdaF,
        csvColSampleUdaB,
        csvColEmpty,
        csvColHallProbeB,
        csvColSampleUac,
        csvColSampleUbd,
        csvColSampleUcd,
        csvColSampleUda,
        /** csvColEnd is number of columns we have not a real column. */
        csvColEnd,
    };
    /** Fully automated measurement in progress */
    bool measRunning;
    /** Array of steps for fully automatized Hall measurement. */
    static const Step_t stepsAll[];
    /** Array of steps for single "hand made" measurement. */
    static const Step_t stepsMeasure[];
    /** Vector of steps to run, created from autoSteps. */
    Steps_t stepsRunning;
    /** Current step of automated Hall measurement. */
    Steps_t::const_iterator stepCurrent;
    /** Dinamic "mark" in hall automation steps for loops. */
    Steps_t::const_iterator stepLoopMark;
    /** Timer used for fully automated testing process. */
    QTimer measTimer;
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
    /** File to save measured data. */
    QCSVFile csvFile;
    /** HP 34970A device to measure voltage and resistivity. */
    HP34970hack hp34970Hack;
    /** User interface widgets */
    Ui::MainWindow *ui;

    /** Close all devices, eg. power supply, Agilent, switch, ... */
    void closeDevs();
    /** Abort measurement. */
    void measureAbort();
    /** Start prepared measurement steps. */
    void measureStart();
    /** Open all devices. */
    bool openDevs();
    /** Read single value from 34901A. */
    double readSingle();
    /* Steps for Hall measurement automation */
    /** Abort process. */
    static bool stepAbort(MainWindow *this_);
    /** Put loop mark, record position where to return with stepLoopIf... . */
    static bool stepCreateLoopMark(MainWindow *this_);
    /** Finish measurement, write data into file etc. . */
    static bool stepFinish(MainWindow *this_);
    /** Get current time and put in into measurement data. */
    static bool stepGetTime(MainWindow *this_);
    /** Open all routes (clean up). */
    static bool stepOpenAllRoutes(MainWindow *this_);
    /** Prepare measurement on hall probe. */
    static bool stepMeasHallProbePrepare(MainWindow *this_);
    /** Do measurement on hall probe. */
    static bool stepMeasHallProbe(MainWindow *this_);

    static bool stepSampleMeas_cd(MainWindow *this_);
    static bool stepSampleMeas_cdRev(MainWindow *this_);
    static bool stepSampleMeas_da(MainWindow *this_);
    static bool stepSampleMeas_daRev(MainWindow *this_);
    static bool stepSampleMeas_ac(MainWindow *this_);
    static bool stepSampleMeas_acRev(MainWindow *this_);
    static bool stepSampleMeas_bd(MainWindow *this_);
    static bool stepSampleMeas_bdRev(MainWindow *this_);
    /** Prepare measurement on sample pins c, d. */
    static bool stepSampleMeasPrepare_cd(MainWindow *this_);
    /** Prepare measurement on sample pins d, a. */
    static bool stepSampleMeasPrepare_da(MainWindow *this_);
    /** Prepare measurement on sample pins a, c. */
    static bool stepSampleMeasPrepare_ac(MainWindow *this_);
    /** Prepare measurement on sample pins b, d. */
    static bool stepSampleMeasPrepare_bd(MainWindow *this_);
    /** Put power on sample on pins (b, a) = (+, -) */
    static bool stepSamplePower_ba(MainWindow *this_);
    /** Put power on sample on pins (b, c) = (+, -) */
    static bool stepSamplePower_bc(MainWindow *this_);
    /** Put power on sample on pins (b, d) = (+, -) */
    static bool stepSamplePower_bd(MainWindow *this_);
    /** Put power on sample on pins (c, a) = (+, -) */
    static bool stepSamplePower_ca(MainWindow *this_);
    /** Set sample power source power. */
    static bool stepSamplePower_mp(MainWindow *this_);
    /** Set sample power source power (reversed). */
    static bool stepSamplePower_pm(MainWindow *this_);
};

#endif // MAINWINDOW_H
