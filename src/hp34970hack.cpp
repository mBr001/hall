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

QString HP34970hack::readCmd()
{
    QString s;

    s = sendQuery("READ?");
    return s;
}

void HP34970hack::setChannel(int channel, bool open)
{
    // ROUT:SCAN (@101,103,104) pro nastaveni z ktereho portu read? čte hodnoty
}

void HP34970hack::setup()
{
    sendCmd("CONF:VOLT (@101:104)");
    sendCmd("ROUT:CLOS (@201:206,209,210)");
    sendCmd("INIT", 2000000);
}

