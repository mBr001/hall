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
    void on_coilCurrDoubleSpinBox_valueChanged(double );
    void on_coilCurrMaxDoubleSpinBox_valueChanged(double );
    void on_coilCurrMinDoubleSpinBox_valueChanged(double );
    void on_coilCurrStepDoubleSpinBox_valueChanged(double );
    void on_coilPowerCheckBox_toggled(bool checked);
    void on_experiment_fatalError(const QString &errorShort, const QString &errorLong);
    void on_experiment_measured(double B, double hallU, double resistivity,
                                double resistivitySpec, double errAsymetry, double errShottky);
    void on_experiment_measurementCompleted();
    void on_experiment_sweepingCompleted();
    void on_hallProbeDeleteToolButton_clicked();
    void on_hallProbeNameComboBox_currentIndexChanged(const QString &arg1);
    void on_hallProbeSaveToolButton_clicked();
    void on_measurePushButton_clicked();
    void on_sampleCurrDoubleSpinBox_valueChanged(double );
    void on_sampleNameLineEdit_editingFinished();
    void on_sampleThicknessDoubleSpinBox_valueChanged(double );
    void on_startPushButton_clicked();

private:
    /** Application configuration. */
    Config config;
    /** Configuration dialog. */
    ConfigUI configUI;
    /** Experiment driving class. */
    Experiment experiment;

    QVector<double> dataB;
    QVector<double> dataHallU;
    QVector<double> dataResistivity;
    QwtPlotCurve qwtPlotCurveHallU;
    QwtPlotCurve qwtPlotCurveResistivity;

    Ui::MainWindow *ui;

    void close();
    /** Start prepared measurement steps. */
    void measure(bool single);
};

#endif // MAINWINDOW_H
