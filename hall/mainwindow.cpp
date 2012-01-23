#include <QCloseEvent>
#include <QMessageBox>
#include <qwt_symbol.h>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "unitconv.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    config(),
    configUI(&config),
    experiment(&config, this),
    qwtPlotCurveHallU("Hall U"),
    qwtPlotCurveResistivity("Resistivity"),
    ui(new Ui::MainWindow)
{
    experiment.setObjectName("experiment");
    ui->setupUi(this);
    QObject::connect(&configUI, SIGNAL(accepted()), this, SLOT(show()));
    QObject::connect(&experiment, SIGNAL(coilBMeasured(double)),
                     ui->coilBDoubleSpinBox, SLOT(setValue(double)));
    QObject::connect(&experiment, SIGNAL(coilIMeasured(double)),
                     ui->coilCurrMeasDoubleSpinBox, SLOT(setValue(double)));
    QObject::connect(&experiment, SIGNAL(coilUMeasured(double)),
                     ui->coilVoltMeasDoubleSpinBox, SLOT(setValue(double)));

    ui->qwtPlot->enableAxis(QwtPlot::yRight, true);
    QString titleTpl("<html><body><span style=\"font-family:'Sans Serif'; font-size:14pt; font-weight:600; font-style:bold;\"><span style=\"%1\">%2</span> %3</span></body></html>");
    QwtText titleR(titleTpl.arg("color: lime;").arg("*").arg("Resistivity [Ω]"));
    ui->qwtPlot->setAxisTitle(QwtPlot::yRight, titleR);

    QwtText titleHall(titleTpl.arg("color: red;").arg("×").arg("hall U [V]"));
    ui->qwtPlot->setAxisTitle(QwtPlot::yLeft, titleHall);
    ui->qwtPlot->setAxisTitle(QwtPlot::xBottom, "B [T]");
    qwtPlotCurveHallU.attach(ui->qwtPlot);
    qwtPlotCurveHallU.setStyle(QwtPlotCurve::NoCurve);

    QwtSymbol *qwtPlotHallUSymbol = new QwtSymbol(QwtSymbol::XCross);
    qwtPlotHallUSymbol->setColor(QColor(255, 0, 0));
    qwtPlotHallUSymbol->setSize(QSize(12, 12));
    qwtPlotCurveHallU.setSymbol(qwtPlotHallUSymbol);

    qwtPlotCurveResistivity.setYAxis(QwtPlot::yRight);
    qwtPlotCurveResistivity.attach(ui->qwtPlot);
    qwtPlotCurveResistivity.setStyle(QwtPlotCurve::NoCurve);

    QwtSymbol *qwtPlotResistivitySymbol = new QwtSymbol(QwtSymbol::Star1);
    qwtPlotResistivitySymbol->setColor(QColor(0, 255, 0));
    qwtPlotResistivitySymbol->setSize(QSize(12, 12));
    qwtPlotCurveResistivity.setSymbol(qwtPlotResistivitySymbol);

    ui->dataTableWidget->resizeColumnsToContents();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::close()
{
    experiment.close();
    config.setCoilIRangeMax(ui->coilCurrMaxDoubleSpinBox->value());
    config.setCoilIRangeMin(ui->coilCurrMinDoubleSpinBox->value());
    config.setCoilIRangeStep(ui->coilCurrStepDoubleSpinBox->value());
    config.setSampleI(UnitConv::fromDisplay(ui->sampleCurrDoubleSpinBox->value(), sampleIUnits));
    hide();
    configUI.show();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (configUI.isHidden() && configUI.result() == QDialog::Accepted) {
        event->ignore();
        if (experiment.coilI() != 0.) {
            if (QMessageBox::warning(
                        this, "Power is still on!",
                        "Power is still on and should be turned (slowly!) "
                        "off before end of experiment.\n\n"
                        "Exit experiment withought shutdown?",
                        QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes) {
                // TODO: Offer shut down.
                return;
            }
        }
        close();
        return;
    }

    QMainWindow::closeEvent(event);
}

QString MainWindow::doubleToString(double x)
{
    return QString("%1").arg(x, 0, 'E', 4);
}

void MainWindow::doStartMeasure(bool single)
{
    experiment.measure(single);

    ui->startStopStackedWidget->setCurrentIndex(STOP_STACK);

    ui->automaticGroupBox->setEnabled(false);
    ui->coilGroupBox->setEnabled(false);
    ui->manualGroupBox->setEnabled(false);
}

void MainWindow::on_abortMeasurementPushButton_clicked()
{
    experiment.measurementAbort();
}

void MainWindow::on_coilCurrDoubleSpinBox_valueChanged(double value)
{
    if (ui->coilPowerCheckBox->isChecked()) {
        ui->startStopStackedWidget->setEnabled(false);
        ui->sweepingProgressBar->setMaximum(0);
        ui->sweepingWidget->setEnabled(true);

        experiment.setCoilI(value);
    }
}

void MainWindow::on_coilCurrMaxDoubleSpinBox_valueChanged(double val1)
{
    double val2(ui->coilCurrMinDoubleSpinBox->value());
    experiment.setCoilIRange(val1, val2);
}

void MainWindow::on_coilCurrMinDoubleSpinBox_valueChanged(double val1)
{
    double val2(ui->coilCurrMaxDoubleSpinBox->value());
    experiment.setCoilIRange(val1, val2);
}

void MainWindow::on_coilPowerCheckBox_toggled(bool checked)
{
    ui->startStopStackedWidget->setEnabled(true);
    ui->sweepingProgressBar->setMaximum(0);
    ui->sweepingWidget->setEnabled(true);

    experiment.setCoilI(checked ? ui->coilCurrDoubleSpinBox->value() : 0.);
}

void MainWindow::on_experiment_fatalError(const QString &errorShort, const QString &errorLong)
{
    QString title("Fatal error in experiment: ");
    QString text("%1:\n\n%2");

    text = text.arg(errorShort).arg(errorLong);
    title.append(errorShort);
    QMessageBox::critical(this, title, text);
    close();
}

void MainWindow::on_experiment_measured(const HallData::MeasuredData &,
                                        const HallData::EvaluatedData &evaluatedData)
{
    ui->dataTableWidget->insertRow(0);

    ui->dataTableWidget->setItem(
                0, 0, new QTableWidgetItem(doubleToString(evaluatedData.B)));
    ui->dataTableWidget->setItem(
                0, 1, new QTableWidgetItem(doubleToString(evaluatedData.Uhall)));
    ui->dataTableWidget->setItem(
                0, 2, new QTableWidgetItem(doubleToString(UnitConv::toDisplay(config.sampleI(), sampleIUnits))));
    ui->dataTableWidget->setItem(
                0, 3, new QTableWidgetItem(doubleToString(evaluatedData.R)));
    ui->dataTableWidget->setItem(
                0, 4, new QTableWidgetItem(doubleToString(UnitConv::toDisplay(evaluatedData.Rspec, resistivitySpecUnits))));
    ui->dataTableWidget->setItem(
                0, 5, new QTableWidgetItem(doubleToString(UnitConv::toDisplay(evaluatedData.carrierConcentration, carriercUnits))));
    ui->dataTableWidget->setItem(
                0, 6, new QTableWidgetItem(doubleToString(evaluatedData.driftSpeed)));
    ui->dataTableWidget->setItem(
                0, 7, new QTableWidgetItem(QVariant(round(evaluatedData.errAsymetry * 1000.) / 10.).toString()));
    ui->dataTableWidget->setItem(
                0, 8, new QTableWidgetItem(QVariant(round(evaluatedData.errShottky * 1000.) / 10.).toString()));
    ui->dataTableWidget->resizeColumnsToContents();

    //ui->carriercLineEdit->setText(doubleToString(carrierConc / carriercUnit));

    if (isfinite(evaluatedData.B)) {
        ui->coilBDoubleSpinBox->setValue(evaluatedData.B);

        // TODO: skip NAN data from ploting

        const QVector<double> &dataB(experiment.data().B());
        const double *dataX = dataB.constData();
        const int dataSize = dataB.size();
        qwtPlotCurveResistivity.setRawSamples(
                    dataX, experiment.data().R().constData(), dataSize);
        qwtPlotCurveHallU.setRawSamples(dataX, experiment.data().Uhall().constData(), dataSize);
        ui->qwtPlot->replot();
    }
    else {
        ui->coilBDoubleSpinBox->setValue(ui->coilBDoubleSpinBox->minimum());
        ui->statusBar->showMessage("Warning: NAN in data found! ", 5000);
    }
}

void MainWindow::on_experiment_measurementCompleted()
{
    ui->automaticGroupBox->setEnabled(true);
    ui->coilGroupBox->setEnabled(true);
    ui->manualGroupBox->setEnabled(true);
    ui->startStopStackedWidget->setCurrentIndex(START_STACK);
    ui->coilCurrDoubleSpinBox->setValue(experiment.coilI());
    ui->coilPowerCheckBox->setChecked(experiment.coilI() != 0.);
}

void MainWindow::on_experiment_sweepingCompleted()
{
    ui->startStopStackedWidget->setEnabled(true);
    ui->sweepingProgressBar->setMaximum(100);
    ui->sweepingWidget->setEnabled(false);
}

void MainWindow::on_startManualPushButton_clicked()
{
    if (!experiment.isMeasuring())
        doStartMeasure(true);
}

void MainWindow::on_sampleCurrDoubleSpinBox_valueChanged(double value)
{
    config.setSampleI(UnitConv::fromDisplay(value, sampleIUnits));
}

void MainWindow::on_startAutomaticPushButton_clicked()
{
    if (!experiment.isMeasuring()) {
        config.setCoilIRangeStep(ui->coilCurrStepDoubleSpinBox->value());
        doStartMeasure(false);
    }
}

void MainWindow::reset()
{
    ui->automaticGroupBox->setEnabled(true);
    ui->coilGroupBox->setEnabled(true);
    ui->manualGroupBox->setEnabled(true);
    ui->startStopStackedWidget->setCurrentIndex(START_STACK);
    ui->carriercLineEdit->setText("N/A");
    ui->resistivityLineEdit->setText("N/A");
    ui->resistivitySpecLineEdit->setText("N/A");
    ui->driftLineEdit->setText("N/A");
    ui->dataTableWidget->clearContents();
}

void MainWindow::show()
{
    if (!experiment.open())
        return;

    double val;

    val = experiment.coilI();
    ui->coilCurrDoubleSpinBox->setValue(val);
    ui->coilPowerCheckBox->setChecked(val != 0);

    ui->sampleCurrDoubleSpinBox->setValue(UnitConv::toDisplay(config.sampleI(), sampleIUnits));

    val = experiment.coilMaxI();
    ui->coilCurrDoubleSpinBox->setMaximum(val);
    ui->coilCurrDoubleSpinBox->setMinimum(-val);
    ui->coilCurrMaxDoubleSpinBox->setMaximum(val);
    ui->coilCurrMaxDoubleSpinBox->setMinimum(-val);
    ui->coilCurrMinDoubleSpinBox->setMaximum(val);
    ui->coilCurrMinDoubleSpinBox->setMinimum(-val);
    ui->coilCurrStepDoubleSpinBox->setMaximum(val);

    ui->coilCurrMaxDoubleSpinBox->setValue(config.coilIRangeMax());
    ui->coilCurrMinDoubleSpinBox->setValue(config.coilIRangeMin());
    ui->coilCurrStepDoubleSpinBox->setValue(config.coilIRangeStep());

    setWindowTitle(QString("Hall - ") + config.sampleName());
    ui->sampleNameLineEdit->setText(config.sampleName());
    ui->sampleThicknessDoubleSpinBox->setValue(UnitConv::toDisplay(config.sampleThickness(), sampleThicknessUnits));
    ui->sampleHolderLineEdit->setText(config.selectedSampleHolderName());
    ui->notePlainTextEdit->clear();

    reset();

    QWidget::show();
}

void MainWindow::startApp()
{
    configUI.show();
}

void MainWindow::on_resetExperimentPushButton_clicked()
{
    reset();
}
