#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/select.h>
#include <unistd.h>
#include <QtCore>

#include "ps6220hack.h"

PS6220Hack::PS6220Hack() :
    fd(-1)
{
}

void PS6220Hack::close()
{
    if (fd >= 0) {
        ::close(fd);
        fd = -1;
    }
}

double PS6220Hack::current()
{
    QString cmd("curr?\n");
    char buf[256];
    ssize_t size;

    write(fd, cmd.toAscii().constData(), cmd.toAscii().size());

    size = serial_read(buf, sizeof(buf));
    if (size <= 0)
        return -1.;

    buf[size] = 0;
    QString vals(buf);

    vals = vals.trimmed();
    bool ok;
    double val;
    val = QVariant(vals).toDouble(&ok);

    return val;
}

bool PS6220Hack::isline(const char *buf, ssize_t size)
{
    // empty line is not line
    if (size <= 1)
        return false;

    if (buf[size - 1] == '\n' || buf[size - 1] == '\r')
        return true;

    return false;
}

bool PS6220Hack::open(const char *fname)
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

bool PS6220Hack::output()
{
    QString cmd("outp?\n");
    char buf[256];
    ssize_t size;

    write(fd, cmd.toAscii().constData(), cmd.toAscii().size());

    size = serial_read(buf, sizeof(buf));
    if (size <= 0)
        return -1.;

    buf[size] = 0;
    QString vals(buf);

    vals = vals.trimmed();
    bool ok;
    int val;
    val = QVariant(vals).toInt(&ok);

    return val;
}

/**
 * Open serial port and set parameters as defined for SDP power source.
 * @param fname File name of serial port.
 * @return      File descriptor on success, negative number (err no.) on error.
 */
int PS6220Hack::serial_open(const char* fname)
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

/**
 * Reads data from serial port.
 * @param buf   Buffer to store readed data.
 * @param count Maximal amount of bytes to read.
 * @return      Number of bytes succesfully readed, or gefative number
 *      (error no.) on error.
 */
ssize_t PS6220Hack::serial_read(char *buf, ssize_t count)
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

void PS6220Hack::setCurrent(double current)
{
    QString cmd("curr %1\n");

    cmd = cmd.arg(current).replace(",", ".");

    write(fd, cmd.toAscii().constData(), cmd.toAscii().size());
}

void PS6220Hack::setOutput(bool out)
{
    if (out) {
        const char cmd_out_on[] = "OUTP ON\n";

        write(fd, cmd_out_on, sizeof(cmd_out_on) - 1);
    }
    else {
        const char cmd_out_on[] = "OUTP OFF\n";

        write(fd, cmd_out_on, sizeof(cmd_out_on) - 1);
    }
}
