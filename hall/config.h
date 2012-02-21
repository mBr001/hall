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
    QString dataDirPath();
    void deleteSampleHolder(const QString &sampleHolderName);
    double hallProbeCurrent(const QString &sampleHolderName);
    QString hallProbeEquationB(const QString &sampleHolderName);
    QStringList sampleHolders();
    QString hp34970Port();
    QString msdpPort();
    QString polSwitchPort();
    QString ps6220Port();
    double sampleI();
    QString sampleName();
    double sampleThickness();
    QString selectedSampleHolderName();
    void setCoilIRangeMax(double IMax);
    void setCoilIRangeMin(double IMin);
    void setCoilIRangeStep(double IStep);
    void setDataDirPath(const QString &dirPath);
    void setHallProbeCurrent(const QString &sampleHolderName, double I);
    void setHallProbeEquationB(const QString &sampleHolderName, const QString &eqation);
    void setHp34970Port(const QString &port);
    void setMsdpPort(const QString &port);
    void setPolSwitchPort(const QString &port);
    void setPs6220Port(const QString &port);
    void setSampleI(double I);
    void setSampleName(const QString &sampleName);
    void setSampleThickness(double thickness);
    void setSelectedSampleHolderName(const QString &sampleHolderName);

private:
    /* Names of configuration options */
    static const char cfg_coilIRangeMax[];
    static const char cfg_coilIRangeMin[];
    static const char cfg_coilIRangeStep[];
    static const char cfg_dataDirPath[];
    static const char cfg_hallProbe_current_suffix[];
    static const char cfg_hallProbe_equationB_suffix[];
    static const char cfg_hp34970Port[];
    static const char cfg_msdpPort[];
    static const char cfg_ps6220Port[];
    static const char cfg_polSwitchPort[];
    static const char cfg_sampleI[];
    static const char cfg_sampleHolder_prefix[];
    static const char cfg_sampleName[];
    static const char cfg_sampleSize[];
    static const char cfg_sampleThickness[];
    static const char cfg_selectedSampleHolderName[];

    QSettings settings;

    QString buildSampleHolderKeyPrefix(QString sampleHolderName,
                                       bool slash = true);
};

#endif // CONFIG_H
