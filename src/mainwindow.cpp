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
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    configUI = new ConfigUI(this);

    currentTimer.setInterval(100);
    currentTimer.setSingleShot(true);
    QObject::connect(&currentTimer, SIGNAL(timeout()), this,
                     SLOT(on_currentTimer_timeout()));
}

MainWindow::~MainWindow()
{
    delete configUI;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (configUI->exec() == QDialog::Accepted) {
        event->ignore();
        return;
    }

    QMainWindow::close();
}

void MainWindow::on_closePushButton_clicked()
{
    ConfigUI configUI(this);

    if (configUI.exec() != QDialog::Accepted) {
        this->close();
        return;
    }
}

bool MainWindow::openDevs()
{
    QString s;
    int err;

    s = settings.value(ConfigUI::cfg_powerSupplyPort).toString();
    err = sdp_open(&sdp, s.toLocal8Bit().constData(), 0);
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

    /* TODO ... */

    return true;

sdp_err:
    sdp_close(&sdp);

sdp_err0:
    QMessageBox::critical(
        this, "Failed to open Manson SDP power supply.", sdp_strerror(err));
    statusBar()->showMessage(sdp_strerror(err));

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
    if (ui->reverseCheckBox->isChecked())
        current = -current;

    sdp_set_curr(&sdp, current);

    /* TODO */
}

void MainWindow::on_reverseCheckBox_toggled(bool checked)
{
    if (checked)
        ui->polarityLabel->setText(pol_mp);
    else
        ui->polarityLabel->setText(pol_pm);

    currentTimer.start();
}

void MainWindow::on_currentDoubleSpinBox_valueChanged(double)
{
    currentTimer.start();
}


void MainWindow::show()
{
    if (configUI->exec() != QDialog::Accepted)
        return;

    QWidget::show();
}
