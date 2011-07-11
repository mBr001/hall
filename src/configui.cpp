#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include "configui.h"
#include "ui_configui.h"

const char ConfigUI::cfg_agilentPort[] = "Agilent Port";
const char ConfigUI::cfg_fileName[] = "File name";
const char ConfigUI::cfg_powerSupplyPort[] = "Power Supply Port";
const char ConfigUI::cfg_polSwitchPort[] = "Switch Port";
const char ConfigUI::cfg_samplePSPort[] = "Sample PS K6220";

ConfigUI::ConfigUI(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigUI)
{
    ui->setupUi(this);
    QString s;

    s = settings.value(cfg_agilentPort, "GPIB1").toString();
    ui->agilentPortComboBox->setEditText(s);

    s = settings.value(cfg_powerSupplyPort, "COM1").toString();
    ui->powerSupplyPortComboBox->setEditText(s);

    s = settings.value(cfg_polSwitchPort, "LPT1").toString();
    ui->switchPortComboBox->setEditText(s);

    s = settings.value(cfg_samplePSPort, "COM1").toString();
    ui->samplePowerPortComboBox->setEditText(s);

    s = settings.value(cfg_fileName, QString()).toString();
    ui->fileNameLineEdit->setText(s);

}

ConfigUI::~ConfigUI()
{
    delete ui;
}

void ConfigUI::on_buttonBox_accepted()
{
    QString s;

    s = ui->agilentPortComboBox->currentText();
    settings.setValue(cfg_agilentPort, s);

    s = ui->powerSupplyPortComboBox->currentText();
    settings.setValue(cfg_powerSupplyPort, s);

    s = ui->switchPortComboBox->currentText();
    settings.setValue(cfg_polSwitchPort, s);

    s= ui->samplePowerPortComboBox->currentText();
    settings.setValue(cfg_samplePSPort, s);

    s = ui->fileNameLineEdit->text();
    QFile f(s);
    if (f.exists()) {
        QString msg("File \n\n%1\n\n already exists, relly owerwrite this file?");

        msg = msg.arg(s);
        if (QMessageBox::question(
                    this, "File already exists.", msg,
                    QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok)
            return;
        f.remove();
    }
    settings.setValue(cfg_fileName, s);

    this->accept();
}

void ConfigUI::on_buttonBox_rejected()
{
    this->reject();
}

void ConfigUI::on_fileNameToolButton_clicked()
{
    QString fileName;

    fileName = QFileDialog::getSaveFileName(
                this, "File to save measured data.", QString(),
                tr("CSV Files (*.csv);;All files (*)"));
    if (fileName.isEmpty())
        return;
    ui->fileNameLineEdit->setText(fileName);
}
