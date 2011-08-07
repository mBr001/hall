#ifndef CONFIG_H
#define CONFIG_H

#include <QSettings>

class Config
{
public:
    Config();
    QString dataFileName();
    QString hp34970Port();
    QString msdpPort();
    QString polSwitchPort();
    QString ps6220Port();
    void setDataFileName(const QString &port);
    void setHp34970Port(const QString &port);
    void setPolSwitchPort(const QString &port);
    void setPs6220Port(const QString &port);
    void setMsdpPort(const QString &port);

private:
    /* Names of configuration options */
    static const char cfg_hp34970Port[];
    static const char cfg_dataFileName[];
    static const char cfg_ps6220Port[];
    static const char cfg_polSwitchPort[];
    static const char cfg_msdpPort[];

    QSettings settings;
};

#endif // CONFIG_H
