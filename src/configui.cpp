#include "configui.h"
#include "ui_configui.h"

ConfigUI::ConfigUI(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigUI)
{
    ui->setupUi(this);
}

ConfigUI::~ConfigUI()
{
    delete ui;
}

void ConfigUI::on_buttonBox_accepted()
{
    this->accept();
}

void ConfigUI::on_buttonBox_rejected()
{
    this->reject();
}
