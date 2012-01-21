#include <QCloseEvent>
#include <QMessageBox>
#include <qwt_symbol.h>

#include "mainwindow.h"
#include "ui_mainwindow.h"

const double MainWindow::carriercUnit = 1e6;
const double MainWindow::resistivitySpecUnit = 1e-2;
const double MainWindow::sampleIUnit = .001;
const double MainWindow::sampleThicknessUnit = 1e-6;

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
    config.setSampleI(ui->sampleCurrDoubleSpinBox->value() * sampleIUnit);
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

void MainWindow::measure(bool single)
{
    experiment.measure(single);

    ui->measurePushButton->setText("Abort");
    ui->startPushButton->setText("Abort");

    ui->automaticGroupBox->setEnabled(false);
    ui->coilGroupBox->setEnabled(false);
    ui->manualGroupBox->setEnabled(false);
}

void MainWindow::on_coilCurrDoubleSpinBox_valueChanged(double value)
{
    if (ui->coilPowerCheckBox->isChecked()) {
        ui->measurePushButton->setEnabled(false);
        ui->startPushButton->setEnabled(false);
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

    int t(experiment.ETA());
    QTime qt;
    ui->remainsTimeEdit->setTime(qt.addSecs(t));
}

void MainWindow::on_coilCurrStepDoubleSpinBox_valueChanged(double val)
{
    experiment.setCoilIStep(val);
}

void MainWindow::on_coilPowerCheckBox_toggled(bool checked)
{
    ui->measurePushButton->setEnabled(false);
    ui->startPushButton->setEnabled(false);
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

void MainWindow::on_experiment_measured(double B, double hallU, double resistivity,
                                        double resistivitySpec, double carrierConc,
                                        double errAsymentry, double errShottky)
{
    ui->dataTableWidget->insertRow(0);

    ui->dataTableWidget->setItem(
                0, 0, new QTableWidgetItem(doubleToString(B)));
    ui->dataTableWidget->setItem(
                0, 1, new QTableWidgetItem(doubleToString(hallU)));
    ui->dataTableWidget->setItem(
                0, 2, new QTableWidgetItem(doubleToString(config.sampleI() / sampleIUnit)));
    ui->dataTableWidget->setItem(
                0, 3, new QTableWidgetItem(doubleToString(resistivity)));
    ui->dataTableWidget->setItem(
                0, 4, new QTableWidgetItem(doubleToString(resistivitySpec / resistivitySpecUnit)));
    ui->dataTableWidget->setItem(
                0, 5, new QTableWidgetItem(doubleToString(carrierConc / carriercUnit)));
    ui->dataTableWidget->setItem(
                0, 6, new QTableWidgetItem("N/A"));
    ui->dataTableWidget->setItem(
                0, 7, new QTableWidgetItem(QVariant(round(errAsymentry * 1000.) / 10.).toString()));
    ui->dataTableWidget->setItem(
                0, 8, new QTableWidgetItem(QVariant(round(errShottky * 1000.) / 10.).toString()));
    ui->dataTableWidget->resizeColumnsToContents();

    //ui->carriercLineEdit->setText(doubleToString(carrierConc / carriercUnit));

    if (isfinite(B)) {
        ui->coilBDoubleSpinBox->setValue(B);

        // TODO: skip NAN data from ploting

        const QVector<double> &dataB(experiment.getDataB());
        const double *dataX = dataB.constData();
        const int dataSize = dataB.size();
        qwtPlotCurveResistivity.setRawSamples(
                    dataX, experiment.getDataResistivity().constData(), dataSize);
        qwtPlotCurveHallU.setRawSamples(dataX, experiment.getDataHallU().constData(), dataSize);
        ui->qwtPlot->replot();
    }
    else {
        ui->coilBDoubleSpinBox->setValue(
                    ui->coilBDoubleSpinBox->minimum());
        ui->statusBar->showMessage("Warning: NAN in data found! ", 5000);
    }
}

void MainWindow::on_experiment_measurementCompleted()
{
    ui->automaticGroupBox->setEnabled(true);
    ui->coilGroupBox->setEnabled(true);
    ui->manualGroupBox->setEnabled(true);

    ui->startPushButton->setText("Start");
    ui->measurePushButton->setText("Single measurement");

    ui->coilCurrDoubleSpinBox->setValue(experiment.coilI());
    ui->coilPowerCheckBox->setChecked(experiment.coilI() != 0.);
}

void MainWindow::on_experiment_sweepingCompleted()
{
    ui->measurePushButton->setEnabled(true);
    ui->startPushButton->setEnabled(true);
    ui->sweepingProgressBar->setMaximum(100);
    ui->sweepingWidget->setEnabled(false);
}

void MainWindow::on_measurePushButton_clicked()
{
    if (experiment.isMeasuring()) {
        experiment.measurementAbort();
    }
    else {
        measure(true);
    }
}

void MainWindow::on_sampleCurrDoubleSpinBox_valueChanged(double value)
{
    config.setSampleI(value * sampleIUnit);
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

void MainWindow::reset()
{
    ui->automaticGroupBox->setEnabled(true);
    ui->coilGroupBox->setEnabled(true);
    ui->manualGroupBox->setEnabled(true);
    ui->carriercLineEdit->setText("N/A");
    ui->resistivityLineEdit->setText("N/A");
    ui->resistivitySpecLineEdit->setText("N/A");
    ui->driftLineEdit->setText("N/A");
    ui->measurePushButton->setText("Single measurement");
    ui->startPushButton->setText("Start");
    ui->dataTableWidget->clear();
}

void MainWindow::show()
{
    if (!experiment.open())
        return;

    double val;

    val = experiment.coilI();
    ui->coilCurrDoubleSpinBox->setValue(val);
    ui->coilPowerCheckBox->setChecked(val != 0);

    ui->sampleCurrDoubleSpinBox->setValue(config.sampleI() / sampleIUnit);

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
    experiment.setCoilIStep(config.coilIRangeStep());

    setWindowTitle(QString("Hall - ") + config.sampleName());
    ui->sampleNameLineEdit->setText(config.sampleName());
    ui->sampleThicknessDoubleSpinBox->setValue(config.sampleThickness() / sampleThicknessUnit);
    ui->sampleHolderLineEdit->setText(config.selectedSampleHolderName());

    reset();

    QWidget::show();
}

void MainWindow::startApp()
{
    configUI.show();
}
