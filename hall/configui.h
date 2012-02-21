#ifndef CONFIGUI_H
#define CONFIGUI_H

#include <QDialog>
#include "config.h"

namespace Ui {
    class ConfigUI;
}

class ConfigUI : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigUI(Config *config, QWidget *parent = 0);
    ~ConfigUI();

protected:
    static QStringList listParalelPorts();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_dirPathToolButton_clicked();
    void on_sampleHolderAddToolButton_clicked();
    void on_sampleHolderDeleteToolButton_clicked();
    void on_sampleHolderComboBox_editTextChanged(const QString &arg1);
    void on_sampleHolderComboBox_currentIndexChanged(const QString &arg1);

private:
    Ui::ConfigUI *ui;
    Config *config;

    static const double sampleThicknessUnit;
    static const double sampleIUnit;
    static const double sampleHolderCurrentUnit;
};

#endif // CONFIGUI_H
