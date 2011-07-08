#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include "configui.h"
#include "ui_configui.h"

const char ConfigUI::cfg_agilentPort[] = "Agilent Port";
const char ConfigUI::cfg_fileName[] = "File name";
const char ConfigUI::cfg_powerSupplyPort[] = "Power Supply Port";
const char ConfigUI::cfg_switchPort[] = "Switch Port";

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

    s = settings.value(cfg_switchPort, "LPT1").toString();
    ui->switchPortComboBox->setEditText(s);

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
    settings.setValue(cfg_switchPort, s);

    s = ui->fileNameLineEdit->text();
    QFile f(s);
    if (f.exists()) {
        QString msg("File %1 already exists, owerwrite this file?");

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

    fileName = QFileDialog::getOpenFileName(
                this, "File to save measured data.", QString(),
                tr("CSV Files (*.csv);;All files (*)"));
    if (fileName.isEmpty())
        return;
    ui->fileNameLineEdit->setText(fileName);
}
