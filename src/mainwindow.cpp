#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "configui.h"

const char MainWindow::pol_pm[] =
        "<span style='font-weight:600;'><span style='color:#ff0000;'>+</span> <span style='color:#0000ff;'>-</span></span>";
const char MainWindow::pol_mp[] =
        "<span style='font-weight:600;'><span style='color:#0000ff;'>-</span> <span style='color:#ff0000;'>+</span></span>";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    currentOld(0)
{
    ui->setupUi(this);

    currentTimer.setInterval(100);
    currentTimer.setSingleShot(true);
    QObject::connect(&currentTimer, SIGNAL(timeout()), this,
                     SLOT(updateCurrent()));
}

MainWindow::~MainWindow()
{
    delete ui;
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

    s = settings.value(ConfigUI::cfg_powerSupplyPort).toString();
    if (sdp_open(&sdp, s.toLocal8Bit().constData(), 0) < 0)
        return false;

    sdp_va_t setpoint;

    if (sdp_get_va_setpoint(&sdp, &setpoint) < 0) {
        sdp_close(&sdp);
        return false;
    }
    currentOld = setpoint.curr;

    sdp_lcd_info_t lcd_info;
    if (sdp_get_lcd_info(&sdp, &lcd_info) < 0) {
        sdp_close(&sdp);
        return false;
    }
    ui->powerCheckBox->setChecked(lcd_info.output);

    return true;
}

void MainWindow::closeDevs()
{
    sdp_close(&sdp);
}

void MainWindow::on_powerCheckBox_toggled(bool checked)
{
    if (checked) {
        // FIXME
        sdp_set_curr(&sdp, 0);
        sdp_set_output(&sdp, true);
        currentOld = 0;
    }
    currentTimer.start();
}

void MainWindow::on_measurePushButton_clicked()
{

}

void MainWindow::updateCurrent()
{
    double current;

    current = (double)ui->currentSpinBox->value() / 0.001;
    if (ui->reverseCheckBox->isChecked())
        current = -current;

    sdp_set_curr(&sdp, current);


    // sdp_set_output(&sdp, checked);
}

void MainWindow::on_reverseCheckBox_toggled(bool checked)
{
    if (checked)
        ui->polarityLabel->setText(pol_mp);
    else
        ui->polarityLabel->setText(pol_pm);

    currentTimer.start();
}
