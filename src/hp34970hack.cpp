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

// read value
// ROUT:SCAN ()
// READ?
//

// howto:
// conf:res (@101); :conf:volt (@103,104
// ROUT:SCAN (@101,103,104)


/*
procedure THall.ResBtnClick(Sender: TObject);
var s:string;
    kod:integer;
    cur,volp,voln,Ra,Rb,Rs,pomr,t:double;
begin
 ResBtn.Enabled:=false; HallBtn.Enabled:=false;
 OpenChannel('209,210');
 s:=EditCurr.Text; val(s,cur,kod);
 if not(kod=0) then begin
  ShowMessage('Failed to Convert'+#13+'Current to Number');
  exit;
 end;
 s:=EditT.Text; val(s,t,kod);
 if not(kod=0) then begin
  ShowMessage('Failed to Convert'+#13+'Thickness to Number');
  exit;
 end;
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
 s:=EditCurr.Text; val(s,cur,kod);
 if not(kod=0) then begin
  ShowMessage('Failed to Convert'+#13+'Current to Number');
  exit;
 end;
 s:=EditT.Text; val(s,t,kod);
 if not(kod=0) then begin
  ShowMessage('Failed to Convert'+#13+'Thickness to Number');
  exit;
 end;
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

HP34970hack::Sense_t HP34970hack::SenseVolt = "CONF:VOLT";
HP34970hack::Sense_t HP34970hack::SenseRes = "CONF:RES";

HP34970hack::HP34970hack() :
    QSerial()
{
}

HP34970hack::~HP34970hack()
{
    close();
}

void HP34970hack::close()
{
    QSerial::close();
}

void HP34970hack::init()
{
    sendCmd("INIT", 2000000);
}

void HP34970hack::sendCmd(QString cmd, long timeout)
{
    QString s;

    s = sendQuery(cmd, timeout);
    if (!s.isEmpty())
        throw new std::runtime_error("P34970hack::cmd response not empty.");
}

bool HP34970hack::open(QString port)
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

QString HP34970hack::sendQuery(QString cmd, long timeout)
{
    QString s;

    s = cmd.trimmed().append(";*OPC?\n");
    write(s);
    s = readLine(1024, timeout).trimmed();
    if (!s.endsWith('1'))
        throw new std::runtime_error("P34970hack::sendQuery failed read response.");
    if (s.endsWith(";1"))
        return s.left(s.size() - 2);

    return s.left(s.size() - 1);
}

QStringList HP34970hack::read()
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

void HP34970hack::setScan(QList<int> channels)
{
    QStringList ch;

    foreach(int channel, channels)
    {
        ch.append(QVariant(channel).toString());
    }

    QString cmd("ROUT:SCAN (@%1)");

    sendCmd(cmd.arg(ch.join(",")));
}

void HP34970hack::setSense(QList<int> channels, Sense_t sense)
{
    QStringList ch;

    foreach(int channel, channels)
    {
        ch.append(QVariant(channel).toString());
    }

    QString cmd("%1 (@%2)");

    sendCmd(cmd.arg(sense).arg(ch.join(",")));
}

void HP34970hack::routeChannels(QList<int> closeChannels, int offs)
{
    QStringList close, open;

    for (int x(offs + 1); x <= offs + 20; ++x) {
        QVariant channel(x);
        if (closeChannels.contains(x))
            close.append(channel.toString());
        else
            open.append(channel.toString());
    }

    sendCmd(QString("ROUT:OPEN (@%1)").arg(open.join(",")));
    if (!close.empty())
        sendCmd(QString("ROUT:CLOS (@%1)").arg(close.join(",")));
}

void HP34970hack::setup()
{
    QList<int> channels;

    channels << 101 << 102 << 103 << 104 << 114;
    setSense(channels, SenseVolt);
    sendCmd("CONF:VOLT (@101:104,114)");
    init();
}

