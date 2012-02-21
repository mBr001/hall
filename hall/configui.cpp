#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include "configui.h"
#include "../QSCPIDev/qserial.h"
#include "ui_configui.h"

const double ConfigUI::sampleHolderCurrentUnit = 1./1000.;
const double ConfigUI::sampleThicknessUnit = 1./1000000.;
const double ConfigUI::sampleIUnit = 1./1000.;

ConfigUI::ConfigUI(Config *config, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigUI)
{
    this->config = config;
    ui->setupUi(this);
    this->resize(this->width(), 0);

    QStringList ports(QSerial::list());
    ui->agilentPortComboBox->addItems(ports);
    ui->powerSupplyPortComboBox->addItems(ports);
    ui->samplePowerPortComboBox->addItems(ports);
    ui->switchPortComboBox->addItems(listParalelPorts());

    ui->agilentPortComboBox->setEditText(config->hp34970Port());
    ui->dirPathLineEdit->setText(config->dataDirPath());
    ui->powerSupplyPortComboBox->setEditText(config->msdpPort());
    ui->samplePowerPortComboBox->setEditText(config->ps6220Port());
    ui->switchPortComboBox->setEditText(config->polSwitchPort());

    ui->sampleHolderComboBox->addItems(config->sampleHolders());
    QString sampleHolderName(config->selectedSampleHolderName());
    ui->sampleHolderComboBox->setEditText(sampleHolderName);
    ui->hallProbeCurrentDoubleSpinBox->setValue(
                config->hallProbeCurrent(sampleHolderName) / sampleHolderCurrentUnit);
    ui->hallProbeBEquationLineEdit->setText(
                config->hallProbeEquationB(sampleHolderName));
    ui->sampleNameLineEdit->setText(config->sampleName());
    ui->sampleThicknessDoubleSpinBox->setValue(
                config->sampleThickness() / sampleThicknessUnit);
}

ConfigUI::~ConfigUI()
{
    delete ui;
}

QStringList ConfigUI::listParalelPorts()
{
#ifdef __linux__
    QString root("/dev/");
    QDir dir(root);
    QStringList filters;
    filters << "parport*";
    dir.setNameFilters(filters);
    QStringList ports(dir.entryList(QDir::System, QDir::Name | QDir::LocaleAware));
    ports.replaceInStrings(QRegExp("^"), root);
#else
#error "Not implemented!"
#endif
    return ports;
}

void ConfigUI::on_buttonBox_accepted()
{
    QDir dir(ui->dirPathLineEdit->text());
    if (!dir.exists()) {
        QString msg("Data directory does not exists, would you like to create directory: \n"
                    + dir.path());

        if (QMessageBox::question(this, "Create data directory?", msg, QMessageBox::Ok, QMessageBox::Cancel) != QMessageBox::Ok)
            return;
        if (!dir.mkpath(dir.path())) {
            QMessageBox::critical(this, "Failed to create directory.",
                                  QString("Failed to create directory:\n") + dir.path());
            return;
        }
    }
    config->setDataDirPath(dir.path());
    config->setHp34970Port(ui->agilentPortComboBox->currentText());
    config->setMsdpPort(ui->powerSupplyPortComboBox->currentText());
    config->setPolSwitchPort(ui->switchPortComboBox->currentText());
    config->setPs6220Port(ui->samplePowerPortComboBox->currentText());
    config->setSampleName(ui->sampleNameLineEdit->text());
    config->setSampleThickness(
                ui->sampleThicknessDoubleSpinBox->value() * sampleThicknessUnit);
    QString hallProbeName(ui->sampleHolderComboBox->currentText());
    config->setHallProbeCurrent(hallProbeName,
                                ui->hallProbeCurrentDoubleSpinBox->value() * sampleHolderCurrentUnit);
    config->setHallProbeEquationB(hallProbeName,
                                 ui->hallProbeBEquationLineEdit->text());
    config->setSelectedSampleHolderName(hallProbeName);

    this->accept();
}

void ConfigUI::on_buttonBox_rejected()
{
    this->reject();
}

void ConfigUI::on_dirPathToolButton_clicked()
{
    QString dirPath(QFileDialog::getExistingDirectory(
                this, "Select directory to store measurement data.",
                ui->dirPathLineEdit->text()));
    if (!dirPath.isEmpty())
        ui->dirPathLineEdit->setText(dirPath);
}

void ConfigUI::on_sampleHolderAddToolButton_clicked()
{
    QString sampleHolderName(ui->sampleHolderComboBox->currentText());
    config->setHallProbeCurrent(sampleHolderName,
                                ui->hallProbeCurrentDoubleSpinBox->value() * sampleHolderCurrentUnit);
    config->setHallProbeEquationB(sampleHolderName,
                                 ui->hallProbeBEquationLineEdit->text());
    int idx(ui->sampleHolderComboBox->findText(
                sampleHolderName, Qt::MatchExactly | Qt::MatchCaseSensitive));
    if (idx == -1 || ui->sampleHolderComboBox->itemText(idx) != sampleHolderName)
        ui->sampleHolderComboBox->addItem(sampleHolderName);

    ui->sampleHolderDeleteToolButton->setEnabled(true);
}

void ConfigUI::on_sampleHolderComboBox_editTextChanged(const QString &arg1)
{
    ui->sampleHolderDeleteToolButton->setEnabled(
                ui->sampleHolderComboBox->findText(arg1) >= 0);
}

void ConfigUI::on_sampleHolderDeleteToolButton_clicked()
{
    QString sampleHolderName(ui->sampleHolderComboBox->currentText());
    config->deleteSampleHolder(sampleHolderName);
    int idx(ui->sampleHolderComboBox->findText(sampleHolderName));
    ui->sampleHolderComboBox->removeItem(idx);
}

void ConfigUI::on_sampleHolderComboBox_currentIndexChanged(const QString &arg1)
{
    ui->hallProbeBEquationLineEdit->setText(config->hallProbeEquationB(arg1));
    ui->hallProbeCurrentDoubleSpinBox->setValue(config->hallProbeCurrent(arg1) / sampleHolderCurrentUnit);
}
