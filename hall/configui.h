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
    explicit ConfigUI(QWidget *parent = 0);
    ~ConfigUI();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_fileNameToolButton_clicked();

private:

    Ui::ConfigUI *ui;
    Config config;
};

#endif // CONFIGUI_H
