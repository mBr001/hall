#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/select.h>
#include <unistd.h>
#include <QtCore>
#include <stdexcept>

#include "hp34970hack.h"

HP34970Hack::Sense_t HP34970Hack::SenseVolt = "CONF:VOLT";
HP34970Hack::Sense_t HP34970Hack::SenseRes = "CONF:RES";

HP34970Hack::HP34970Hack() :
    QSerial()
{
    memset(HP34903ClosedChannels, 0, sizeof(HP34903ClosedChannels));
}

HP34970Hack::~HP34970Hack()
{
    close();
}

void HP34970Hack::close()
{
    QSerial::close();
}

QString HP34970Hack::formatCmd(const QString &cmd, const Channels_t &channels)
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

void HP34970Hack::init()
{
    sendCmd("INIT", 2000000);
}

bool HP34970Hack::open(const QString &port)
{
    const long timeout = (10l * 1000000l) / 9600l;

    close();

    if (!QSerial::open(port, QSerial::Baude9600, 300000, timeout))
        return false;

    write("\n");
    sendCmd("*RST;*CLS", 500000);
    sendCmd("SYST:REM");

    return true;
}

QStringList HP34970Hack::read()
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

void HP34970Hack::sendCmd(const QString &cmd, long timeout)
{
    QString s;

    s = sendQuery(cmd, timeout);
    if (!s.isEmpty())
        throw new std::runtime_error("P34970hack::cmd response not empty.");
}

void HP34970Hack::sendCmd(const QString &cmd, const Channels_t &channels, long timeout)
{
    QString s;

    s = sendQuery(cmd, channels, timeout);
    if (!s.isEmpty())
        throw new std::runtime_error("P34970hack::cmd response not empty.");
}

QString HP34970Hack::sendQuery(const QString &cmd, long timeout)
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

QString HP34970Hack::sendQuery(const QString &cmd, const Channels_t &channels, long timeout)
{
    QString _cmd(formatCmd(cmd, channels));

    return sendQuery(_cmd, timeout);
}

void HP34970Hack::setRoute(Channels_t closeChannels, int offs)
{
    Channels_t openChannels;
    Channels_t _closeChannels_;

    // channel = x + 1 + offs;
    ++offs;
    for (int x(0); x <= 19; ++x) {
        bool c(HP34903ClosedChannels[x]);
        Channel_t channel = x + offs;
        bool cw(closeChannels.count(channel));

        if (c && !cw) {
            openChannels.append(channel);
            HP34903ClosedChannels[x] = false;
        } else
        if (!c && cw) {
            _closeChannels_.append(channel);
            HP34903ClosedChannels[x] = true;
        }
    }

    if(!openChannels.isEmpty())
        sendCmd("ROUT:OPEN", openChannels);
    if (!closeChannels.isEmpty())
        sendCmd("ROUT:CLOS", _closeChannels_);
}

void HP34970Hack::setScan(Channel_t channel)
{
    QString cmd("ROUT:SCAN (@%1);:INIT");

    sendCmd(cmd.arg(channel), 2000000);
}

void HP34970Hack::setScan(Channels_t channels)
{
    QString cmd("ROUT:SCAN");

    cmd = formatCmd(cmd, channels).append(";:INIT");
    sendCmd(cmd, 2000000);
}

void HP34970Hack::setSense(Sense_t sense, Channels_t channels)
{
    sendCmd(sense, channels);
}
