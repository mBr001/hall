#include <errno.h>
#include <math.h>
#include <QMessageBox>
#include <QCloseEvent>
#include "mainwindow.h"
#include "ui_mainwindow.h"

const char MainWindow::pol_pm[] =
        "<span style='font-weight:600;'><span style='color:#ff0000;'>+</span> <span style='color:#0000ff;'>-</span></span>";
const char MainWindow::pol_mp[] =
        "<span style='font-weight:600;'><span style='color:#0000ff;'>-</span> <span style='color:#ff0000;'>+</span></span>";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    configUI()
{
    ui->setupUi(this);

    QObject::connect(&configUI, SIGNAL(accepted()), this, SLOT(show()));

    currentTimer.setInterval(currentDwell);
    currentTimer.setSingleShot(false);
    QObject::connect(&currentTimer, SIGNAL(timeout()), this,
                     SLOT(on_currentTimer_timeout()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeDevs()
{
    currentTimer.stop();
    ui->sweppingLabel->setEnabled(false);
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
    ui->sweppingLabel->setEnabled(true);
}

void MainWindow::on_coilPolCrossCheckBox_toggled(bool checked)
{
    ui->sweppingLabel->setEnabled(true);

    if (checked)
        ui->polarityLabel->setText(pol_mp);
    else
        ui->polarityLabel->setText(pol_pm);
}

void MainWindow::on_coilPowerCheckBox_toggled(bool)
{
    ui->sweppingLabel->setEnabled(true);
}

void MainWindow::on_currentTimer_timeout()
{
    sdp_va_data_t va_data;

    // tweak coil current
    while (ui->sweppingLabel->isEnabled()) {
        /** Curent trought coil */
        double wantI, procI;
        /** Coil power state, on/off */
        bool wantCoilPower, procCoilPower;
        /** Coil power switch state direct/cross */
        PwrPolSwitch::state_t wantCoilSwitchState, procCoilSwitchState;

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

        // proc values
        procCoilSwitchState = pwrPolSwitch.polarity();

        sdp_lcd_info_t lcd_info;
        sdp_get_lcd_info(&sdp, &lcd_info); // TODO check
        procCoilPower = lcd_info.output;
        procI = lcd_info.set_A;
        if (procCoilSwitchState == PwrPolSwitch::cross)
            procI = -procI;

        ui->plainTextEdit->appendPlainText(QString("procI: %1, procCoilSwitchState: %2, procCoilPower: %3\n").arg(procI).arg(procCoilSwitchState).arg(procCoilPower));
        ui->plainTextEdit->appendPlainText(QString("wantI: %1, wantCoilSwitchState: %2, wantCoilPower: %3\n").arg(wantI).arg(wantCoilSwitchState).arg(wantCoilPower));

        /* process decision */
        // Target reach, finish job
        if (fabs(procI - wantI) < currentSlope) {
            ui->sweppingLabel->setEnabled(false);
            if (!wantCoilPower && procI <= currentSlope)
                sdp_set_output(&sdp, 0); // TODO check

            break;
        }

        // Need switch polarity?
        if (procCoilSwitchState != wantCoilSwitchState) {
            // Is polarity switch posible? (power is off)
            if (!procCoilPower) {
                pwrPolSwitch.setPolarity(wantCoilSwitchState); // TODO check
                break;
            }

            // Is posible power-off in order to swich polarity?
            if (fabs(procI) < currentSlope) {
                sdp_set_output(&sdp, 0);
                break;
            }

            // set current near to zero before polarity switching
        }

        // want current but power is off -> set power on at current 0.0 A
        if (procCoilPower != wantCoilPower && wantCoilPower) {
            sdp_set_curr(&sdp, 0.0);
            sdp_set_output(&sdp, 1);
            break;
        }

        // power is on, but current neet to be adjusted, do one step
        if (procI > wantI)
            procI -= currentSlope;
        else
            procI += currentSlope;

        sdp_set_curr(&sdp, fabs(procI));

        break;
    }

    if (sdp_get_va_data(&sdp, &va_data) < 0) {
        // TODO
        close();
        return;
    }

    ui->coilCurrMeasDoubleSpinBox->setValue(va_data.curr);
}

void MainWindow::on_measurePushButton_clicked()
{
    /* TODO */
}

void MainWindow::on_sampleCurrDoubleSpinBox_valueChanged(double )
{

}

void MainWindow::on_samplePolCrossCheckBox_toggled(bool )
{

}

void MainWindow::on_samplePowerCheckBox_toggled(bool )
{

}

bool MainWindow::openDevs()
{
    QString err_text, err_title;
    QString s;
    int err;

    s = settings.value(ConfigUI::cfg_powerSupplyPort).toString();
    err = sdp_open(&sdp, s.toLocal8Bit().constData(), SDP_DEV_ADDR_MIN);
    if (err < 0)
        goto sdp_err0;

    /* Set value limit in current input spin  box. */
    sdp_va_t limits;
    err = sdp_get_va_maximums(&sdp, &limits);
    if (err < 0)
        goto sdp_err;
    ui->coilCurrDoubleSpinBox->setMaximum(limits.curr);

    /* Set current to actual value, avoiding anny jumps. */
    sdp_va_data_t va_data;
    err = sdp_get_va_data(&sdp, &va_data);
    if (err < 0)
        goto sdp_err;
    err = sdp_set_curr(&sdp, va_data.curr);
    if (err < 0)
        goto sdp_err;
    /* Set voltage to maximum, Hall is current driven. */
    err = sdp_set_volt_limit(&sdp, va_data.volt);
    if (err < 0)
        goto sdp_err;
    err = sdp_set_volt(&sdp, va_data.volt);
    if (err < 0)
        goto sdp_err;

    sdp_lcd_info_t lcd_info;
    if (err < 0)
        goto sdp_err;
    ui->coilPowerCheckBox->setChecked(lcd_info.output);

    s = settings.value(ConfigUI::cfg_polSwitchPort).toString();
    if (!pwrPolSwitch.open(s.toLocal8Bit().constData())) {
        err = errno;
        goto mag_pwr_switch_err;
    }

    bool cross;
    cross = (pwrPolSwitch.polarity() == PwrPolSwitch::cross);
    ui->coilPolCrossCheckBox->setChecked(cross);

    /* TODO ... */

    ui->sweppingLabel->setEnabled(true);
    currentTimer.start();

    return true;

    // powerSwitch.close();

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

void MainWindow::updateCurrent()
{
    double current;

    current = ui->coilCurrDoubleSpinBox->value();
    if (ui->coilPolCrossCheckBox->isChecked())
        current = -current;

    sdp_set_curr(&sdp, current);

    /* TODO */
}
