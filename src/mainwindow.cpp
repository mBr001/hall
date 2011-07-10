#include <errno.h>
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

    currentTimer.setInterval(500);
    currentTimer.setSingleShot(false);
    QObject::connect(&currentTimer, SIGNAL(timeout()), this,
                     SLOT(on_currentTimer_timeout()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (configUI.isHidden() && configUI.result() == QDialog::Accepted) {
        event->ignore();
        hide();
        configUI.show();

        return;
    }

    QMainWindow::closeEvent(event);
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
    ui->currentDoubleSpinBox->setMaximum(limits.curr);

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
    ui->powerCheckBox->setChecked(lcd_info.output);

    s = settings.value(ConfigUI::cfg_powerSupplyPort).toString();
    if (!powerSwitch.open(s.toLocal8Bit().constData())) {
        err = errno;
        goto mag_pwr_switch_err;
    }

    bool cross;
    cross = powerSwitch.polarity() == PowerSwitch::cross;
    ui->polarityCheckBox->setChecked(cross);

    /* TODO ... */

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

void MainWindow::closeDevs()
{
    sdp_close(&sdp);
}

void MainWindow::on_powerCheckBox_toggled(bool checked)
{
    if (checked) {
        // FIXME
        // sdp_set_curr(&sdp, 0);
        // sdp_set_output(&sdp, true);
    }
    currentTimer.start();
}

void MainWindow::on_measurePushButton_clicked()
{
    /* TODO */
}

void MainWindow::on_currentTimer_timeout()
{
    /* TODO */
}

void MainWindow::updateCurrent()
{
    double current;

    current = ui->currentDoubleSpinBox->value();
    if (ui->polarityCheckBox->isChecked())
        current = -current;

    sdp_set_curr(&sdp, current);

    /* TODO */
}

void MainWindow::on_polarityCheckBox_toggled(bool checked)
{
    if (checked) {
        powerSwitch.setPolarity(PowerSwitch::cross);
        ui->polarityLabel->setText(pol_mp);
    } else {
        powerSwitch.setPolarity(PowerSwitch::direct);
        ui->polarityLabel->setText(pol_pm);
    }

    currentTimer.start();
}

void MainWindow::on_currentDoubleSpinBox_valueChanged(double)
{
    currentTimer.start();
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
