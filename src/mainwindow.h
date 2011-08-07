#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QTimer>

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
    void on_coilPowerCheckBox_toggled(bool checked);
    void on_experiment_measured();
    void on_experiment_measurementCompleted();
    void on_experiment_sweepingCompleted();
    void on_measurePushButton_clicked();
    void on_sampleCurrDoubleSpinBox_valueChanged(double );
    void on_startPushButton_clicked();

private:    
    /** Application configuration */
    Config config;
    /** Configuration dialog. */
    ConfigUI configUI;
    /** Experiment driving class. */
    Experiment experiment;
    /** User interface widgets */
    Ui::MainWindow *ui;

    /** Start prepared measurement steps. */
    void measure(bool single);
};

#endif // MAINWINDOW_H
