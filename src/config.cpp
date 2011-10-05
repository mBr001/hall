#include "config.h"
#include <math.h>

const char Config::cfg_coilIRangeMax[] = "coil I range max";
const char Config::cfg_coilIRangeMin[] = "coil I range min";
const char Config::cfg_coilIRangeStep[] = "coil I range step";
const char Config::cfg_dataFileName[] = "experiment/data file name";
const char Config::cfg_hallProbe[] = "hall probes";
const char Config::cfg_hp34970Port[] = "experiment/HP34970 port";
const char Config::cfg_msdpPort[] = "experiment/coil PS port";
const char Config::cfg_polSwitchPort[] = "experiment/polarity switch port";
const char Config::cfg_ps6220Port[] = "experiment/sample PS port";
const char Config::cfg_sampleI[] = "sample I";
const char Config::cfg_sampleSize[] = "sample size";
const char Config::cfg_sampleThickness[] = "sample thickness";

Config::Config()
{
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

QString Config::dataFileName()
{
    return settings.value(cfg_dataFileName, QString()).toString();
}

void Config::deleteHallProbeEquationB(const QString &name)
{
    settings.beginGroup(cfg_hallProbe);
    settings.remove(name);
    settings.endGroup();
}

QString Config::hallProbeEquationB(const QString &name)
{
    settings.beginGroup(cfg_hallProbe);
    QString equation(settings.value(name, "").toString());
    settings.endGroup();

    return equation;
}

QStringList Config::hallProbes()
{
    settings.beginGroup(cfg_hallProbe);
    QStringList hallProbes(settings.childKeys());
    settings.endGroup();

    hallProbes.replaceInStrings(QRegExp("-[0-9]+$"), QString()).removeDuplicates();

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

double Config::sampleSize()
{
    return settings.value(cfg_sampleSize, 0).toDouble();
}

double Config::sampleThickness()
{
    return settings.value(cfg_sampleThickness, 0).toDouble();
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

void Config::setDataFileName(const QString &port)
{
    settings.setValue(cfg_dataFileName, port);
}

void Config::setHallProbeEquationB(const QString &name, const QString &equation)
{
    settings.beginGroup(cfg_hallProbe);
    settings.setValue(name, equation);
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

void Config::setSampleSize(double size)
{
    settings.setValue(cfg_sampleSize, size);
}

void Config::setSampleThickness(double thickness)
{
    settings.setValue(cfg_sampleThickness, thickness);
}
