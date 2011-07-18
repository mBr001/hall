#include <errno.h>
#include <math.h>
#include <QCloseEvent>
#include <QDateTime>
#include <QMessageBox>
#include <stdexcept>
#include <vector>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#endif

#include "mainwindow.h"
#include "ui_mainwindow.h"

const char MainWindow::pol_pn[] =
        "<span style='font-weight:600;'><span style='color:#ff0000;'>+</span> <span style='color:#0000ff;'>-</span></span>";
const char MainWindow::pol_np[] =
        "<span style='font-weight:600;'><span style='color:#0000ff;'>-</span> <span style='color:#ff0000;'>+</span></span>";

const MainWindow::automationStep_t MainWindow::autoSteps[] = {
    {
       autoStop, 0,
    },
};

const QVector<MainWindow::automationStep_t>
        MainWindow::autoStepsVect((int)ARRAY_SIZE(MainWindow::autoSteps), MainWindow::autoSteps[0]);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    autoRunning(false),
    configUI(),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    QObject::connect(&configUI, SIGNAL(accepted()), this, SLOT(show()));

    coilTimer.setInterval(currentDwell);
    coilTimer.setSingleShot(false);
    QObject::connect(&coilTimer, SIGNAL(timeout()), this,
                     SLOT(on_currentTimer_timeout()));

    automationTimer.setSingleShot(true);
    QObject::connect(&automationTimer, SIGNAL(timeout()), this,
                     SLOT(on_automationTimer_timeout()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::auto00(MainWindow *this_)
{
    this_->hp34970Hack.setChannelOpen(209, true);
    this_->hp34970Hack.setChannelOpen(210, true);

    return true;
}

bool MainWindow::auto01(MainWindow *this_)
{
    this_->ps622Hack.setCurrent(this_->ui->sampleCurrDoubleSpinBox->value());

    return true;
}

bool MainWindow::auto02(MainWindow *this_)
{
    return false;
}

bool MainWindow::auto03(MainWindow *this_)
{
    return false;
}

bool MainWindow::auto04(MainWindow *this_)
{
    return false;
}

bool MainWindow::auto05(MainWindow *this_)
{
    return false;
}

bool MainWindow::auto06(MainWindow *this_)
{
    return false;
}

bool MainWindow::auto07(MainWindow *this_)
{
    return false;
}

bool MainWindow::auto08(MainWindow *this_)
{
    return false;
}

bool MainWindow::auto09(MainWindow *this_)
{
    return false;
}

bool MainWindow::auto10(MainWindow *this_)
{
    return false;
}

bool MainWindow::autoMark(MainWindow *this_)
{
    this_->autoStepMark = this_->autoStepCurrent + 1;

    return true;
}

bool MainWindow::autoStop(MainWindow *this_)
{
    return false;
}

void MainWindow::on_automationTimer_timeout()
{
    // TODO ...
}

void MainWindow::closeDevs()
{
    coilTimer.stop();
    ui->sweepingWidget->setEnabled(false);
    csvFile.close();
    hp34970Hack.close();
    ps622Hack.close();
    pwrPolSwitch.close();
    sdp_close(&sdp);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (configUI.isHidden() && configUI.result() == QDialog::Accepted) {
        closeDevs();
        event->ignore();
        hide();
        configUI.show();

        return;
    }

    QMainWindow::closeEvent(event);
}

void MainWindow::on_coilCurrDoubleSpinBox_valueChanged(double )
{
    ui->sweepingWidget->setEnabled(true);
}

void MainWindow::on_coilPolCrossCheckBox_toggled(bool checked)
{
    ui->sweepingWidget->setEnabled(true);

    if (checked)
        ui->polarityLabel->setText(pol_np);
    else
        ui->polarityLabel->setText(pol_pn);
}

void MainWindow::on_coilPowerCheckBox_toggled(bool)
{
    ui->sweepingWidget->setEnabled(true);
}

void MainWindow::on_currentTimer_timeout()
{
    sdp_lcd_info_t lcd_info;

    if (sdp_get_lcd_info(&sdp, &lcd_info) < 0) {
        throw new std::runtime_error(
                "on_currentTimer_timeout - sdp_get_lcd_info");
        return;
    }

    ui->coilCurrMeasDoubleSpinBox->setValue(lcd_info.read_A);
    ui->coilVoltMeasDoubleSpinBox->setValue(lcd_info.read_V);

    // update coil current 
    if (!ui->sweepingWidget->isEnabled())
        return;

    /** Curent trought coil */
    double procI, wantI;
    /** Coil power state, on/off */
    bool procCoilPower, wantCoilPower;
    /** Coil power switch state direct/cross */
    PwrPolSwitch::state_t procCoilSwitchState, wantCoilSwitchState;

    /* Get all values necesary for process decisions. */
    // wanted values
    if (ui->coilPolCrossCheckBox->isChecked())
        wantCoilSwitchState = PwrPolSwitch::cross;
    else
        wantCoilSwitchState = PwrPolSwitch::direct;

    wantCoilPower = ui->coilPowerCheckBox->isChecked();

    if (wantCoilPower) {
        wantI = ui->coilCurrDoubleSpinBox->value();
        if (wantCoilSwitchState == PwrPolSwitch::cross)
            wantI = -wantI;
    }
    else
        wantI = 0;

    // process values
    procCoilSwitchState = pwrPolSwitch.polarity();

    procCoilPower = lcd_info.output;
    procI = lcd_info.set_A;
    if (procCoilSwitchState == PwrPolSwitch::cross)
        procI = -procI;

    ui->plainTextEdit->appendPlainText(QString(
                "procI: %1, procCoilSwitchState: %2, procCoilPower: %3")
            .arg(procI).arg(procCoilSwitchState).arg(procCoilPower));
    ui->plainTextEdit->appendPlainText(QString(
                "wantI: %1, wantCoilSwitchState: %2, wantCoilPower: %3\n")
            .arg(wantI).arg(wantCoilSwitchState).arg(wantCoilPower));

    /* Make process decision. */
    // Target reach, finish job
    if (fabs(procI - wantI) < currentSlope) {
        ui->sweepingWidget->setEnabled(false);
        if (!wantCoilPower && fabs(procI) <= currentSlope && procCoilPower) {
            if ( sdp_set_output(&sdp, 0) < 0)
                throw new std::runtime_error("timer - sdp_set_output");
        }

        return;
    }

    // Need switch polarity?
    if (procCoilSwitchState != wantCoilSwitchState) {
        // Is polarity switch posible? (power is off)
        if (!procCoilPower) {
            pwrPolSwitch.setPolarity(wantCoilSwitchState);
            return;
        }

        // Is posible power-off in order to swich polarity?
        if (fabs(procI) < currentSlope) {
            sdp_set_output(&sdp, 0);
            return;
        }

        // set current near to zero before polarity switching
    }

    // want current but power is off -> set power on at current 0.0 A
    if (procCoilPower != wantCoilPower && wantCoilPower) {
        sdp_set_curr(&sdp, 0.0);
        sdp_set_output(&sdp, 1);
        return;
    }

    // power is on, but current neet to be adjusted, do one step
    if (procI > wantI)
        procI -= currentSlope;
    else
        procI += currentSlope;

    sdp_set_curr(&sdp, fabs(procI));
}

/** Column separator. */
const char cellSeparator[] = ",";
/** Decimal separator used in file for floating point numbers. */
const char decimalSeparator[] = ".";
/** Decimal separator specified by locale settings. */
const char localDecimalSeparator[] = ",";

static QString csvRowAppendColumn(QString row, double val)
{
    /** CSV cell template. */
    QString cell("%1");

    cell = cell.arg(val);
    if (localDecimalSeparator != decimalSeparator)
        cell = cell.replace(decimalSeparator, localDecimalSeparator);

    if (row.isEmpty())
        return cell;

    return QString("%1%2%3").arg(row).arg(cellSeparator).arg(cell);
}

static QString csvRowAppendColumn(QString row, QString cell)
{
    cell = cell.replace("\"", "\"\"");
    if (cell.contains(cellSeparator))
        cell = QString("\"%1\"").arg(cell);

    if (row.isEmpty())
        return cell;

    return row + cellSeparator + cell;
}

void MainWindow::on_measurePushButton_clicked()
{
    QString csvRow;
    QString s;
    double val;

    ui->dataTableWidget->insertRow(0);

    s = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    csvRow = csvRowAppendColumn(csvRow, s);
    ui->dataTableWidget->setItem(0, 3, new QTableWidgetItem(s));

    val = ui->coilCurrMeasDoubleSpinBox->value();
    csvRow = csvRowAppendColumn(csvRow, val);
    s = ui->coilCurrMeasDoubleSpinBox->text();
    ui->dataTableWidget->setItem(0, 0, new QTableWidgetItem(s));

    val = ui->coilVoltMeasDoubleSpinBox->value();
    csvRow = csvRowAppendColumn(csvRow, val);

    val = ui->coilCurrDoubleSpinBox->value();
    csvRow = csvRowAppendColumn(csvRow, val);

    val = ui->sampleCurrDoubleSpinBox->value();
    csvRow = csvRowAppendColumn(csvRow, val);
    s = ui->sampleCurrDoubleSpinBox->text();
    ui->dataTableWidget->setItem(0, 1, new QTableWidgetItem(s));

    s = hp34970Hack.readCmd();
    QStringList cells(s.split(","));
    for (QStringList::const_iterator cell(cells.begin());
                cell != cells.end();
                ++cell)
        csvRow = csvRowAppendColumn(csvRow, *cell);
    ui->plainTextEdit->appendPlainText(s);
    // TODO: inset hall U

    csvFile.write(csvRow.toUtf8());
}

void MainWindow::on_sampleCurrDoubleSpinBox_valueChanged(double value)
{
    ps622Hack.setCurrent(value);
}

void MainWindow::on_samplePolCrossCheckBox_toggled(bool )
{

}

void MainWindow::on_samplePowerCheckBox_toggled(bool checked)
{
    ps622Hack.setOutput(checked);
}

bool MainWindow::openDevs()
{
    QString csvHeader("Time,"
                      "coil curr. meas. [A],"
                      "coil volt. meas. [V],"
                      "coil curr want. [A],"
                      "sample curr. want. [A],"
                      "sample hall [V]"
                      "\r\n");
    /** Text and title shown in error message box */
    QString err_text, err_title;
    QString s;
    int err;

    s = settings.value(ConfigUI::cfg_powerSupplyPort).toString();
    err = sdp_open(&sdp, s.toLocal8Bit().constData(), SDP_DEV_ADDR_MIN);
    if (err < 0)
        goto sdp_err0;

    /* Set value limit in current input spin box. */
    sdp_va_t limits;
    err = sdp_get_va_maximums(&sdp, &limits);
    if (err < 0)
        goto sdp_err;
    ui->coilCurrDoubleSpinBox->setMaximum(limits.curr);

    /* Set actual current value as wanted value, avoiding unwanted hickups. */
    sdp_va_data_t va_data;
    err = sdp_get_va_data(&sdp, &va_data);
    if (err < 0)
        goto sdp_err;

    ui->coilCurrDoubleSpinBox->setValue(va_data.curr);
    err = sdp_set_curr(&sdp, va_data.curr);
    if (err < 0)
        goto sdp_err;

    /* Set voltage to maximum, we drive only current. */
    err = sdp_set_volt_limit(&sdp, limits.volt);
    if (err < 0)
        goto sdp_err;

    err = sdp_set_volt(&sdp, limits.volt);
    if (err < 0)
        goto sdp_err;

    sdp_lcd_info_t lcd_info;
    sdp_get_lcd_info(&sdp, &lcd_info); // TODO check
    if (err < 0)
        goto sdp_err;
    ui->coilPowerCheckBox->setChecked(lcd_info.output);

    // Open polarity switch device
    s = settings.value(ConfigUI::cfg_polSwitchPort).toString();
    if (!pwrPolSwitch.open(s.toLocal8Bit().constData())) {
        err = errno;
        goto mag_pwr_switch_err;
    }

    bool cross;
    cross = (pwrPolSwitch.polarity() == PwrPolSwitch::cross);
    ui->coilPolCrossCheckBox->setChecked(cross);

    // Open sample power source
    s = settings.value(ConfigUI::cfg_samplePSPort).toString();
    if (!ps622Hack.open(s.toLocal8Bit().constData()))
    {
        err = errno;
        goto sample_pwr_err;
    }

    ui->sampleCurrDoubleSpinBox->setValue(ps622Hack.current());
    ui->samplePowerCheckBox->setChecked(ps622Hack.output());

    // Open CSV file to save data
    s = settings.value(ConfigUI::cfg_fileName).toString();
    csvFile.setFileName(s);
    if (!csvFile.open(QFile::WriteOnly | QFile::Truncate))
        goto file_err;

    csvFile.write(csvHeader.toUtf8());

    // Open and setup HP34970 device
    s = settings.value(ConfigUI::cfg_agilentPort).toString();
    if (!hp34970Hack.open(s)) {
        err = errno;
        goto hp34970hack_err;
    }
    hp34970Hack.setup();

    ui->sweepingWidget->setEnabled(true);
    coilTimer.start();

    return true;

    // hp34970Hack.close();

hp34970hack_err:
    if (err_title.isEmpty()) {
        err_title = QString::fromLocal8Bit(
                    "Failed to open HP34970 device");
        err_text = QString::fromLocal8Bit(strerror(err));
    }

file_err:
    ps622Hack.close();
    if (err_title.isEmpty()) {
        err_title = QString::fromLocal8Bit(
                    "Failed to open output file");
        err_text = csvFile.errorString();
    }

sample_pwr_err:
    pwrPolSwitch.close();
    if (err_title.isEmpty()) {
        err_title = QString::fromLocal8Bit(
                    "Failed to open sample power supply (Keithaly 6220)");
        err_text = QString::fromLocal8Bit(strerror(err));
    }

mag_pwr_switch_err:
    if (err_title.isEmpty()) {
        err_title = QString::fromLocal8Bit(
                    "Failed to open power supply switch");
        err_text = QString::fromLocal8Bit(strerror(err));
    }

sdp_err:
    sdp_close(&sdp);

sdp_err0:
    if (err_title.isEmpty()) {
        err_title = QString::fromLocal8Bit(
                    "Failed to open Manson SDP power supply");
        err_text = QString::fromLocal8Bit(sdp_strerror(err));
    }

    err_text = QString("%1:\n\n%2").arg(err_title).arg(err_text);
    QMessageBox::critical(this, err_title, err_text);
    statusBar()->showMessage(err_title);

    return false;
}

void MainWindow::show()
{
    if (!openDevs()) {
        configUI.show();

        return;
    }

    QWidget::show();
}

void MainWindow::startApp()
{
    configUI.show();
}


void MainWindow::on_startPushButton_clicked()
{
    ui->coilGroupBox->setEnabled(autoRunning);
    ui->sampleGroupBox->setEnabled(autoRunning);
    autoRunning = !autoRunning;
    if (autoRunning)
        ui->startPushButton->setText("Stop");
    else
        ui->startPushButton->setText("Stop");
}
