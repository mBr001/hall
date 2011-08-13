#include <QCloseEvent>
#include <QDateTime>
#include <QMessageBox>
#include <qwt_symbol.h>
#include <stdexcept>
#include <vector>

#include "error.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

const double sampleIUnit = 1000.;
const double sampleThicknessUnit = 1000000.;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    config(),
    configUI(),
    experiment(this),
    pointsHallU(),
    pointsResistivity(),
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
    pointsHallU.reserve(1024);
    pointsResistivity.reserve(1024);

    ui->qwtPlot->enableAxis(QwtPlot::yRight, true);
    ui->qwtPlot->setAxisTitle(QwtPlot::yRight, tr("(*) Resistivity [Ω]"));
    ui->qwtPlot->setAxisTitle(QwtPlot::yLeft, "(×) hall U [V]");
    ui->qwtPlot->setAxisTitle(QwtPlot::xBottom, "B [T]");
    qwtPlotCurveHallU.attach(ui->qwtPlot);
    QwtSymbol symbol(QwtSymbol::XCross, qwtPlotCurveHallU.brush(),
            qwtPlotCurveHallU.pen(), QSize(8, 8));
    //symbol.setColor(QColor(255, 0, 0));
    qwtPlotCurveHallU.setStyle(QwtPlotCurve::NoCurve);
    qwtPlotCurveHallU.setSymbol(symbol);

    qwtPlotCurveResistivity.setYAxis(QwtPlot::yRight);
    qwtPlotCurveResistivity.attach(ui->qwtPlot);
    qwtPlotCurveResistivity.setStyle(QwtPlotCurve::NoCurve);
    symbol.setStyle(QwtSymbol::Star1);
    qwtPlotCurveResistivity.setSymbol(symbol);

    ui->hallProbeNameComboBox->addItems(config.hallProbes());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (configUI.isHidden() && configUI.result() == QDialog::Accepted) {
        event->ignore();
        if (experiment.coilI() != 0) {
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
        experiment.close();
        config.setCoilIRangeMax(ui->coilCurrMaxDoubleSpinBox->value());
        config.setCoilIRangeMin(ui->coilCurrMinDoubleSpinBox->value());
        config.setCoilIRangeStep(ui->coilCurrStepDoubleSpinBox->value());
        config.setSampleI(ui->sampleCurrDoubleSpinBox->value());
        config.setSampleThickness(ui->sampleThicknessDoubleSpinBox->value()/sampleThicknessUnit);
        hide();
        configUI.show();
        return;
    }

    QMainWindow::closeEvent(event);
}

void MainWindow::measure(bool single)
{
    experiment.measure(single);
    ui->coilGroupBox->setEnabled(false);
    ui->measurePushButton->setEnabled(false);
    ui->sampleGroupBox->setEnabled(false);
    ui->startPushButton->setText("Abort");
}

void MainWindow::on_coilCurrDoubleSpinBox_valueChanged(double value)
{
    if (ui->coilPowerCheckBox->isChecked()) {
        ui->sweepingWidget->setEnabled(true);
        ui->measurePushButton->setEnabled(false);
        ui->startPushButton->setEnabled(false);

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

void MainWindow::on_coilCurrStepDoubleSpinBox_valueChanged(double val)
{
    experiment.setCoilIStep(val);
}

void MainWindow::on_coilPowerCheckBox_toggled(bool checked)
{
    ui->sweepingWidget->setEnabled(true);
    ui->measurePushButton->setEnabled(false);
    ui->startPushButton->setEnabled(false);
    if (checked) {
        experiment.setCoilI(ui->coilCurrDoubleSpinBox->value());
    }
    else {
        experiment.setCoilI(0);
    }
}

void MainWindow::on_experiment_measured(const QString &time, double B,
                                        double resistivity, double hallU)
{
    ui->dataTableWidget->insertRow(0);

    ui->dataTableWidget->setItem(
                0, 0, new QTableWidgetItem(QVariant(B).toString()));
    ui->dataTableWidget->setItem(
                0, 1, new QTableWidgetItem(QVariant(resistivity).toString()));
    ui->dataTableWidget->setItem(
                0, 2, new QTableWidgetItem(QVariant(hallU).toString()));
    ui->dataTableWidget->setItem(
                0, 3, new QTableWidgetItem(time));
    if (pointsHallU.size() == pointsHallU.capacity()) {
        int reserve(pointsHallU.capacity() * 2);
        pointsHallU.reserve(reserve);
        pointsResistivity.reserve(reserve);
    }

    if (!isnan(B)) {
        ui->coilBDoubleSpinBox->setValue(B);
        if (!isnan(resistivity)) {
            pointsResistivity.append(QPointF(B, resistivity));
            qwtPlotCurveResistivity.setData(pointsResistivity);
        }
        if (!isnan(hallU))
            pointsHallU.append(QPointF(B, hallU));
            qwtPlotCurveHallU.setData(pointsHallU);
        ui->qwtPlot->replot();
    }
}

void MainWindow::on_experiment_measurementCompleted()
{
    ui->coilGroupBox->setEnabled(true);
    ui->sampleGroupBox->setEnabled(true);
    ui->measurePushButton->setEnabled(true);
    ui->startPushButton->setText("Start");
}

void MainWindow::on_experiment_sweepingCompleted()
{
    ui->sweepingWidget->setEnabled(false);
    ui->measurePushButton->setEnabled(true);
    ui->startPushButton->setEnabled(true);
}

void MainWindow::on_measurePushButton_clicked()
{
    measure(true);
}

void MainWindow::on_hallProbeAddToolButton_clicked()
{
    double B1(ui->hallB1DoubleSpinBox->value());
    double B2(ui->hallB2DoubleSpinBox->value());
    double B3(ui->hallB3DoubleSpinBox->value());
    QString name(ui->hallProbeNameComboBox->currentText());

    if (ui->hallProbeNameComboBox->findText(name) == -1) {
        ui->hallProbeNameComboBox->addItem(name);
    }

    config.setHallProbeBn(name, 1, B1);
    config.setHallProbeBn(name, 2, B2);
    config.setHallProbeBn(name, 3, B3);

    experiment.setCoefficients(B1, B2, B3);
}

void MainWindow::on_hallProbeNameComboBox_currentIndexChanged(const QString &arg1)
{
    double B1(config.hallProbeBn(arg1, 1));
    double B2(config.hallProbeBn(arg1, 2));
    double B3(config.hallProbeBn(arg1, 3));

    ui->hallB1DoubleSpinBox->setValue(B1);
    ui->hallB2DoubleSpinBox->setValue(B2);
    ui->hallB3DoubleSpinBox->setValue(B3);

    experiment.setCoefficients(B1, B2, B3);
}


void MainWindow::on_sampleCurrDoubleSpinBox_valueChanged(double value)
{
    experiment.setSampleI(value/sampleIUnit);
}

void MainWindow::on_sampleThicknessDoubleSpinBox_valueChanged(double value)
{
    experiment.setSampleThickness(value/sampleThicknessUnit);
}

void MainWindow::on_startPushButton_clicked()
{
    if (experiment.isMeasuring()) {
        experiment.measurementAbort();
    }
    else {
        // TODO
        //experiment.measurementStartIntervall();
        measure(false);
    }
}

void MainWindow::show()
{
    try {
        experiment.open();
    }
    catch(Error &e)
    {
        QString err_title(e.description());
        QString err_text("%1:\n\n%2");

        err_text = err_text.arg(err_title).arg(e.longDescription());
        QMessageBox::critical(this, err_title, err_text);
        statusBar()->showMessage(err_title);

        configUI.show();

        return;
    }

    double val;

    val = experiment.coilI();
    ui->coilCurrDoubleSpinBox->setValue(val);
    ui->coilPowerCheckBox->setChecked(val != 0);

    val = experiment.sampleI();
    ui->sampleCurrDoubleSpinBox->setValue(val*sampleIUnit);

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
    ui->sampleCurrDoubleSpinBox->setValue(config.sampleI());
    ui->sampleThicknessDoubleSpinBox->setValue(config.sampleThickness()*sampleThicknessUnit);
    experiment.setCoilIStep(ui->coilCurrStepDoubleSpinBox->value());
    QWidget::show();
}

void MainWindow::startApp()
{
    configUI.show();
}
