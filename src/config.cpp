#include "config.h"
#include <math.h>

const char Config::cfg_hallProbe[] = "hall probes";
const char Config::cfg_hp34970Port[] = "experiment/HP34970 port";
const char Config::cfg_dataFileName[] = "experiment/data file name";
const char Config::cfg_msdpPort[] = "experiment/coil PS port";
const char Config::cfg_polSwitchPort[] = "experiment/polarity switch port";
const char Config::cfg_ps6220Port[] = "experiment/sample PS port";

Config::Config()
{
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

QString Config::dataFileName()
{
    return settings.value(cfg_dataFileName, QString()).toString();
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
