#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/select.h>
#include <unistd.h>
#include <QtCore>
#include <stdexcept>

#include "scpi_dev.h"

ScpiDev::Sense_t ScpiDev::SenseVolt = "CONF:VOLT";
ScpiDev::Sense_t ScpiDev::SenseRes = "CONF:RES";

ScpiDev::ScpiDev() :
    QSerial()
{
}

ScpiDev::~ScpiDev()
{
    close();
}

void ScpiDev::close()
{
    QSerial::close();
}

double ScpiDev::current()
{
    QString current(sendQuery("SOUR:CURR?"));

    return QVariant(current).toDouble();
}

QString ScpiDev::formatCmd(const QString &cmd, const Channels_t &channels)
{
    if (!channels.size()) {
        return cmd;
    }

    QStringList ch;
    foreach(int channel, channels) {
        ch.append(QVariant(channel).toString());
    }

    QString format("%1 (@%2)");

    return format.arg(cmd).arg(ch.join(","));
}

void ScpiDev::init()
{
    sendCmd("INIT", 2000000);
}

bool ScpiDev::open(const QString &port, BaudeRate_t baudeRate)
{
    const long timeout = (10l * 1000000l) / 9600l;

    close();

    if (!QSerial::open(port, baudeRate, 300000, timeout))
        return false;

    write("\n");
    sendCmd("*RST;*CLS", 500000);
    sendCmd("SYST:REM");
    routeChannelsClosed.clear();

    return true;
}

bool ScpiDev::output()
{
    QString out(sendQuery("OUTP?"));

    if (out == "1")
        return true;
    if (out == "0")
        return false;

    throw new std::runtime_error("HP34970Hack::output");
}

QStringList ScpiDev::read()
{
    QString s;
    QStringList data;

    s = sendQuery("READ?");
    data = s.split(",", QString::SkipEmptyParts);

    for (QStringList::iterator idata(data.begin()); idata != data.end(); ++idata) {
        *idata = idata->trimmed();
    }

    return data;
}

void ScpiDev::sendCmd(const QString &cmd, long timeout)
{
    QString s;

    s = sendQuery(cmd, timeout);
    if (!s.isEmpty())
        throw new std::runtime_error("HP34970Hack::cmd response not empty.");
}

void ScpiDev::sendCmd(const QString &cmd, const Channels_t &channels, long timeout)
{
    QString s;

    s = sendQuery(cmd, channels, timeout);
    if (!s.isEmpty())
        throw new std::runtime_error("HP34970Hack::cmd response not empty.");
}

QString ScpiDev::sendQuery(const QString &cmd, long timeout)
{
    QString _cmd(cmd + ";*OPC?\n");
    write(_cmd);

    QString result(readLine(1024, timeout).trimmed());
    if (result == "1")
        return QString();
    if (result.endsWith(";1"))
        return result.left(result.size() - 2);

    throw new std::runtime_error("P34970hack::sendQuery failed read response.");
}

QString ScpiDev::sendQuery(const QString &cmd, const Channels_t &channels, long timeout)
{
    QString _cmd(formatCmd(cmd, channels));

    return sendQuery(_cmd, timeout);
}

void ScpiDev::setCurrent(double current)
{
    QString cmd("SOUR:CURR %1");

    sendCmd(cmd.arg(current));
}

void ScpiDev::setOutput(bool enabled)
{
    if (enabled)
        sendCmd("OUTP 1");
    else
        sendCmd("OUTP 0");
}

void ScpiDev::setRoute(Channels_t closeChannels)
{
    Channels_t openChannels(routeChannelsClosed);
    Channels_t closedChannels(closeChannels);

    foreach (Channel_t ch, closeChannels) {
        openChannels.removeOne(ch);
    }

    foreach (Channel_t ch, routeChannelsClosed) {
        closeChannels.removeOne(ch);
    }

    if(!openChannels.isEmpty())
        sendCmd("ROUT:OPEN", openChannels);
    if (!closeChannels.isEmpty())
        sendCmd("ROUT:CLOS", closeChannels);

    routeChannelsClosed = closedChannels;
}

void ScpiDev::setScan(Channel_t channel)
{
    QString cmd("ROUT:SCAN (@%1);:INIT");

    sendCmd(cmd.arg(channel), 2000000);
}

void ScpiDev::setScan(Channels_t channels)
{
    QString cmd("ROUT:SCAN");

    cmd = formatCmd(cmd, channels).append(";:INIT");
    sendCmd(cmd, 2000000);
}

void ScpiDev::setSense(Sense_t sense, Channels_t channels)
{
    sendCmd(sense, channels);
}
