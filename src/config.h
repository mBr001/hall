#ifndef CONFIG_H
#define CONFIG_H

#include <QSettings>
#include <QStringList>

class Config
{
public:
    Config();
    double coilIRangeMax();
    double coilIRangeMin();
    double coilIRangeStep();
    QString dataFileName();
    double hallProbeBn(const QString &name, int idx);
    QStringList hallProbes();
    QString hp34970Port();
    QString msdpPort();
    QString polSwitchPort();
    QString ps6220Port();
    double sampleI();
    double sampleThickness();
    void setCoilIRangeMax(double IMax);
    void setCoilIRangeMin(double IMin);
    void setCoilIRangeStep(double IStep);
    void setDataFileName(const QString &port);
    void setHallProbeBn(const QString &name, int idx, double Bn);
    void setHp34970Port(const QString &port);
    void setMsdpPort(const QString &port);
    void setPolSwitchPort(const QString &port);
    void setPs6220Port(const QString &port);
    void setSampleI(double I);
    void setSampleThickness(double thickness);

private:
    /* Names of configuration options */
    static const char cfg_coilIRangeMax[];
    static const char cfg_coilIRangeMin[];
    static const char cfg_coilIRangeStep[];
    static const char cfg_dataFileName[];
    static const char cfg_hallProbe[];
    static const char cfg_hp34970Port[];
    static const char cfg_msdpPort[];
    static const char cfg_ps6220Port[];
    static const char cfg_polSwitchPort[];
    static const char cfg_sampleI[];
    static const char cfg_sampleThickness[];

    QSettings settings;
};

#endif // CONFIG_H
