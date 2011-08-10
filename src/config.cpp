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

double Config::hallProbeBn(const QString &name, int idx)
{
    QString _name_(name + "-" + QVariant(idx).toString());
    settings.beginGroup(cfg_hallProbe);
    double Bn(settings.value(_name_, NAN).toDouble());
    settings.endGroup();

    return Bn;
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

void Config::setHallProbeBn(const QString &name, int idx, double Bn)
{
    QString _name_(name + "-" + QVariant(idx).toString());
    settings.beginGroup(cfg_hallProbe);
    settings.setValue(_name_, Bn);
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

void Config::setSampleThickness(double thickness)
{
    settings.setValue(cfg_sampleThickness, thickness);
}
