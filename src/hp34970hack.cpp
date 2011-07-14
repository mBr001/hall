#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/select.h>
#include <unistd.h>
#include <QtCore>

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
    fd(-1)
{
}

HP34970hack::~HP34970hack()
{
    close();
}

void HP34970hack::close()
{
    if (fd >= 0) {
        ::close(fd);
        fd = -1;
    }
}

bool HP34970hack::isline(const char *buf, ssize_t size)
{
    // empty line is not line
    if (size <= 1)
        return false;

    if (buf[size - 1] == '\n' || buf[size - 1] == '\r')
        return true;

    return false;
}

bool HP34970hack::open(const char *fname)
{
    const char cmd_rem[] ="syst:rem\n";

    close();

    if (serial_open(fname) < 0)
        goto err;

    write(fd, cmd_rem, sizeof(cmd_rem) - 1);

    return true;

err:

    return false;
}

/**
 * Reads data from serial port.
 * @param buf   Buffer to store readed data.
 * @param count Maximal amount of bytes to read.
 * @return      Number of bytes succesfully readed, or gefative number
 *      (error no.) on error.
 */
ssize_t HP34970hack::serial_read(char *buf, ssize_t count)
{
        const char *buf_ = buf;
        fd_set readfds;
        int ret;
        ssize_t size = 0;
        struct timeval timeout;

        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        // TODO: check this value
        timeout.tv_sec = 0;
        // (bytes * 10 * usec) / bitrate + delay_to_reaction;
        timeout.tv_usec = (count * 10l * 1000000l) / 9600l + 200000l;
        do {
                ssize_t size_;

                ret = select(fd + 1, &readfds, NULL, NULL, &timeout);
                if (ret <= 0) {
                        return -1;
                }
                size_ = read(fd, buf, count);
                if (size_ < 0)
                        return -1;
                size += size_;
                count -= size_;
                buf += size_;
                if (isline(buf_, size))
                        return size;
        } while (count > 0);

        errno = ERANGE;
        return -1;
}

/**
 * Open serial port and set parameters as defined for SDP power source.
 * @param fname File name of serial port.
 * @return      File descriptor on success, negative number (err no.) on error.
 */
int HP34970hack::serial_open(const char* fname)
{
        struct termios tio;

        fd = ::open(fname, O_RDWR | O_NONBLOCK);
        if (fd < 0)
                return -1;

        memset(&tio, 0, sizeof(tio));
        tio.c_cflag = CS8 | CREAD | CLOCAL;
        tio.c_cc[VMIN] = 1;
        tio.c_cc[VTIME] = 5;
        cfsetispeed(&tio, B19200);
        cfsetospeed(&tio, B19200);

        if (tcsetattr(fd, TCSANOW, &tio) < 0) {
                int e = errno;
                ::close(fd);
                errno = e;

                return -1;
        }

        return fd;
}

void HP34970hack::setChannel(int channel, bool open)
{

}

void HP34970hack::setup()
{
    // conf:res (@101); :conf:volt (@103,104
    // ROUT:SCAN (@101,103,104)
    QString cmd("conf:volt (@)\n");

    QString cmd_close("closexxx");
    // CloseChannel101:104
    cmd = cmd.arg("current").replace(",", ".");

    write(fd, cmd.toAscii().constData(), cmd.toAscii().size());
}

