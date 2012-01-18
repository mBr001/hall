#include "config.h"
#include <math.h>

const char Config::cfg_coilIRangeMax[] = "coil I range max";
const char Config::cfg_coilIRangeMin[] = "coil I range min";
const char Config::cfg_coilIRangeStep[] = "coil I range step";
const char Config::cfg_dataDirPath[] = "experiment/data dir path";
const char Config::cfg_hallProbe_equationB[] = "equation B";
const char Config::cfg_hp34970Port[] = "experiment/HP34970 port";
const char Config::cfg_msdpPort[] = "experiment/coil PS port";
const char Config::cfg_polSwitchPort[] = "experiment/polarity switch port";
const char Config::cfg_ps6220Port[] = "experiment/sample PS port";
const char Config::cfg_sampleHolder_prefix[] = "sample holder ";
const char Config::cfg_sampleI[] = "sample I";
const char Config::cfg_sampleName[] = "sample name";
const char Config::cfg_sampleSize[] = "sample size";
const char Config::cfg_sampleThickness[] = "sample thickness";
const char Config::cfg_selectedSampleHolderName[] = "selected sample holder name";

Config::Config()
{
}

QString Config::buildSampleHolderKeyPrefix(QString sampleHolderName,
                                           bool slash)
{
    QString s(QString(cfg_sampleHolder_prefix) + sampleHolderName.replace("/", "_"));

    return slash ? (s + "/") : s;
}

double Config::coilIRangeMax()
{
    return settings.value(cfg_coilIRangeMax, 0).toDouble();
}

double Config::coilIRangeMin()
{
    return settings.value(cfg_coilIRangeMin, 0).toDouble();
}

double Config::coilIRangeStep()
{
    return settings.value(cfg_coilIRangeStep, 0).toDouble();
}

QString Config::dataDirPath()
{
    return settings.value(cfg_dataDirPath, QString()).toString();
}

void Config::deleteSampleHolder(const QString &sampleHolderName)
{
    QString keyPrefix(buildSampleHolderKeyPrefix(sampleHolderName));

    foreach(QString key, settings.allKeys()) {
        if (key.startsWith(keyPrefix))
            settings.remove(key);
    }
}

QString Config::hallProbeEquationB(const QString &sampleHolderName)
{
    settings.beginGroup(buildSampleHolderKeyPrefix(sampleHolderName, false));
    QString equation(settings.value(cfg_hallProbe_equationB, "").toString());
    settings.endGroup();

    return equation;
}

QStringList Config::sampleHolders()
{
    QStringList hallProbes;

    foreach(QString key, settings.allKeys()) {
        if (key.startsWith(cfg_sampleHolder_prefix)) {
            const int from(sizeof(cfg_sampleHolder_prefix) - 1);
            hallProbes.append(key.mid(from, key.indexOf(QChar('/'), from) - from));
        }
    }

    hallProbes.removeDuplicates();
    return hallProbes;
}

QString Config::hp34970Port()
{
    return settings.value(cfg_hp34970Port, QString()).toString();
}

QString Config::msdpPort()
{
    return settings.value(cfg_msdpPort, QString()).toString();
}

QString Config::polSwitchPort()
{
    return settings.value(cfg_polSwitchPort, QString()).toString();
}

QString Config::ps6220Port()
{
    return settings.value(cfg_ps6220Port, QString()).toString();
}

double Config::sampleI()
{
    return settings.value(cfg_sampleI, 0).toDouble();
}

QString Config::sampleName()
{
    return settings.value(cfg_sampleName).toString();
}

double Config::sampleThickness()
{
    return settings.value(cfg_sampleThickness, 0).toDouble();
}

QString Config::selectedSampleHolderName()
{
    return settings.value(cfg_selectedSampleHolderName).toString();
}

void Config::setCoilIRangeMax(double IMax)
{
    settings.setValue(cfg_coilIRangeMax, IMax);
}

void Config::setCoilIRangeMin(double IMin)
{
    settings.setValue(cfg_coilIRangeMin, IMin);
}

void Config::setCoilIRangeStep(double IStep)
{
    settings.setValue(cfg_coilIRangeStep, IStep);
}

void Config::setDataDirPath(const QString &dirPath)
{
    settings.setValue(cfg_dataDirPath, dirPath);
}

void Config::setHallProbeEquationB(const QString &sampleHolderName, const QString &equation)
{
    settings.beginGroup(buildSampleHolderKeyPrefix(sampleHolderName));
    settings.setValue(cfg_hallProbe_equationB, equation);
    settings.endGroup();
}

void Config::setHp34970Port(const QString &port)
{
    settings.setValue(cfg_hp34970Port, port);
}

void Config::setPolSwitchPort(const QString &port)
{
    settings.setValue(cfg_polSwitchPort, port);
}

void Config::setPs6220Port(const QString &port)
{
    settings.setValue(cfg_ps6220Port, port);
}

void Config::setMsdpPort(const QString &port)
{
    settings.setValue(cfg_msdpPort, port);
}

void Config::setSampleI(double I)
{
    settings.setValue(cfg_sampleI, I);
}

void Config::setSampleName(const QString & sampleName)
{
    settings.setValue(cfg_sampleName, sampleName);
}

void Config::setSampleThickness(double thickness)
{
    settings.setValue(cfg_sampleThickness, thickness);
}

void Config::setSelectedSampleHolderName(const QString &sampleHolderName)
{
    settings.setValue(cfg_selectedSampleHolderName, sampleHolderName);
}
