#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include "configui.h"
#include "ui_configui.h"

ConfigUI::ConfigUI(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigUI)
{
    ui->setupUi(this);

    ui->agilentPortComboBox->setEditText(config.hp34970Port());
    ui->powerSupplyPortComboBox->setEditText(config.msdpPort());
    ui->switchPortComboBox->setEditText(config.polSwitchPort());
    ui->samplePowerPortComboBox->setEditText(config.ps6220Port());
    ui->fileNameLineEdit->setText(config.dataFileName());
}

ConfigUI::~ConfigUI()
{
    delete ui;
}

void ConfigUI::on_buttonBox_accepted()
{
    config.setHp34970Port(ui->agilentPortComboBox->currentText());
    config.setMsdpPort(ui->powerSupplyPortComboBox->currentText());
    config.setPs6220Port(ui->samplePowerPortComboBox->currentText());
    config.setPolSwitchPort(ui->switchPortComboBox->currentText());

    QFile f(ui->fileNameLineEdit->text());
    if (f.exists()) {
        QString msg("File \n\n%1\n\n already exists, relly owerwrite this file?");

        msg = msg.arg(f.fileName());
        if (QMessageBox::question(
                    this, "File already exists.", msg,
                    QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok)
            return;
        f.remove();
    }
    config.setDataFileName(f.fileName());

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
