#ifndef CONFIGUI_H
#define CONFIGUI_H

#include <QDialog>
#include <QSettings>

namespace Ui {
    class ConfigUI;
}

class ConfigUI : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigUI(QWidget *parent = 0);
    ~ConfigUI();

    static const char cfg_agilentPort[];
    static const char cfg_powerSupplyPort[];
    static const char cfg_switchPort[];
    static const char cfg_fileName[];

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

    void on_fileNameToolButton_clicked();

private:

    Ui::ConfigUI *ui;
    QSettings settings;
};

#endif // CONFIGUI_H
