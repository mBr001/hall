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

// set DC voltage
// SENS:VOLT:DC ()
// 'SENS:VOLT:DC:RANG'+qs+', (@'+IntToStr(chan)+')';
// SENS:VOLT:DC:NPLC'+qs+', (@'+IntToStr(chan)+')'


/*
procedure THall.ResBtnClick(Sender: TObject);
var s:string;
    kod:integer;
    cur,volp,voln,Ra,Rb,Rs,pomr,t:double;
begin
 ResBtn.Enabled:=false; HallBtn.Enabled:=false;
 OpenChannel('209,210');
 // Convert'+#13+'Current to Number
 s:=EditCurr.Text; val(s,cur,kod);
 // Convert'+#13+'Thickness to Number
 s:=EditT.Text; val(s,t,kod);
 t:=t/10;
 cur:=cur/1000; str(cur:5:3,s);
 { --- Measure R_a --- }
 SetCurrent(s);
 CloseChannel('201,202'); KPausa(300);
 ApplyCurrent(True);
 KPausa(500);
 volp:=-ReadValue(101);
 ApplyCurrent(False);
 str(-cur:6:4,s);
 SetCurrent(s);  KPausa(300);
 ApplyCurrent(True);
 KPausa(500);
 voln:=-ReadValue(101);
 ApplyCurrent(False);
 OpenChannel('201');
 Ra:=(volp-voln)/2/cur*t; str(Ra:7:5,s);
 EditRa.Text:=s;
 { --- Measure R_b --- }
 str(cur:6:4,s);
 SetCurrent(s);
 CloseChannel('203,205'); KPausa(300);
 ApplyCurrent(True);
 KPausa(500);
 volp:=ReadValue(102);
 ApplyCurrent(False);
 str(-cur:6:4,s);
 SetCurrent(s);  KPausa(300);
 ApplyCurrent(True);
 KPausa(500);
 voln:=ReadValue(102);
 ApplyCurrent(False);
 OpenChannel('202,203,205');
 Rb:=(volp-voln)/2/cur*t; str(Rb:7:5,s);
 EditRb.Text:=s;
 { --- Calculate R_s --- }
 Rs:=SolveRs(Ra,Rb,kod);   //(Ra+Rb)/2; kod:=0;
 if not(kod=0) then ShowMessage('Failed on Calculating Rs');
 str(Rs:7:5,s);
 EditRs.Text:=s;
 { --- Measure Field --- }
 CloseChannel('209,210');
 SetCurrent('0.001'); KPausa(300);
 ApplyCurrent(true);
 KPausa(500);
 pomr:=ReadValue(114)*1000;
 pomr:=-30.588+sqrt(934.773+0.392163*pomr);
 ApplyCurrent(false);
 str(pomr:4:2,s);
 EditB.Text:=s;
 ResBtn.Enabled:=true; HallBtn.Enabled:=true;
end;

procedure THall.HallBtnClick(Sender: TObject);
var s:string;
    kod:integer;
    cur,volp,voln,Ra,Rb,Rs,pomr,t:double;
begin
 ResBtn.Enabled:=false; HallBtn.Enabled:=false;
 OpenChannel('209,210');
 // Convert'+#13+'Current to Number
 s:=EditCurr.Text; val(s,cur,kod);
 // Convert'+#13+'Thickness to Number
 s:=EditT.Text; val(s,t,kod);
 t:=t/10;
 cur:=cur/1000; str(cur:6:4,s);
 { --- Measure R_b --- }
 SetCurrent(s);
 CloseChannel('202,204'); KPausa(300);
 ApplyCurrent(True);
 KPausa(500);
 volp:=ReadValue(104);
 ApplyCurrent(False);
 str(-cur:6:4,s);
 SetCurrent(s);  KPausa(300);
 ApplyCurrent(True);
 KPausa(500);
 voln:=ReadValue(104);
 ApplyCurrent(False);
 OpenChannel('202,204');
 Rb:=(volp-voln)/2/cur*t;
 s:=Format('%6.3e',[Rb]);
 Delete(s,Length(s)-2,2);
 EditVb.Text:=s;
 { --- Measure R_a --- }
 str(cur:6:4,s);
 SetCurrent(s);
 CloseChannel('201,203,206'); KPausa(300);
 ApplyCurrent(True);
 KPausa(500);
 volp:=ReadValue(103);
 ApplyCurrent(False);
 str(-cur:6:4,s);
 SetCurrent(s);  KPausa(300);
 ApplyCurrent(True);
 KPausa(500);
 voln:=ReadValue(103);
 ApplyCurrent(False);
 OpenChannel('201,203,206');
 Ra:=(volp-voln)/2/cur*t;
 s:=Format('%6.3e',[Ra]);
 Delete(s,Length(s)-2,2);
 EditVa.Text:=s;
 { --- Measure Field --- }
 CloseChannel('209,210');
 SetCurrent('0.001'); KPausa(300);
 ApplyCurrent(true);
 KPausa(500);
 pomr:=ReadValue(114)*1000;
 pomr:=-30.588+sqrt(934.773+0.392163*pomr);
 ApplyCurrent(false);
 str(pomr:4:2,s);
 EditB.Text:=s;
 { --- Calculate V/B/I --- }
 Rs:=Ra/pomr*10000;
 s:=Format('%6.3e',[Rs]);
 Delete(s,Length(s)-2,2);
 EditUB.Text:=s;
 ResBtn.Enabled:=true; HallBtn.Enabled:=true;
end;
*/

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

void HP34970Hack::setScan(Channels_t channels)
{
    QString cmd("ROUT:SCAN");

    cmd = formatCmd(cmd, channels).append(";:INIT");

    sendCmd(cmd);
}

void HP34970Hack::setSense(Sense_t sense, Channels_t channels)
{
    sendCmd(sense, channels);
}
