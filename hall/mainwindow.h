#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qwt_plot_curve.h>

#include "configui.h"
#include "experiment.h"

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
    void on_abortMeasurementPushButton_clicked();
    void on_coilCurrDoubleSpinBox_valueChanged(double );
    void on_coilCurrMaxDoubleSpinBox_valueChanged(double );
    void on_coilCurrMinDoubleSpinBox_valueChanged(double );
    void on_coilCurrStepDoubleSpinBox_valueChanged(double );
    void on_coilPowerCheckBox_toggled(bool checked);
    void on_experiment_fatalError(const QString &errorShort, const QString &errorLong);
    void on_experiment_measured(double B, double hallU, double resistivity,
                                double resistivitySpec, double carrierConc,
                                double drift,
                                double errAsymetry, double errShottky);
    void on_experiment_measurementCompleted();
    void on_experiment_sweepingCompleted();
    void on_sampleCurrDoubleSpinBox_valueChanged(double );
    void on_startAutomaticPushButton_clicked();
    void on_startManualPushButton_clicked();

private:
    typedef enum {
        START_STACK = 0,
        STOP_STACK = 1
    } StartStopStack_t;

    /** Application configuration. */
    Config config;

    /** Configuration dialog. */
    ConfigUI configUI;

    /** Experiment driving class. */
    Experiment experiment;

    QwtPlotCurve qwtPlotCurveHallU;
    QwtPlotCurve qwtPlotCurveResistivity;

    Ui::MainWindow *ui;

    void close();

    /** Print double to string usign app. specific number formating. */
    QString doubleToString(double x);

    /** Start prepared measurement steps. */
    void doStartMeasure(bool single);
    void reset();

    static const double carriercUnit;
    static const double resistivitySpecUnit;
    static const double sampleIUnit;
    static const double sampleThicknessUnit;
};

#endif // MAINWINDOW_H
