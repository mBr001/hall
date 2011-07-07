#ifndef CONFIGUI_H
#define CONFIGUI_H

#include <QDialog>

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

private:
    Ui::ConfigUI *ui;
};

#endif // CONFIGUI_H
