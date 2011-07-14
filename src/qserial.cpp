#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/select.h>
#include <unistd.h>
#include <QtCore>
#include <exception>

#include "qserial.h"


QSerialError(error)

const QSerial::BaudeRate_t QSerial::Baude9200 = B9600;
const QSerial::BaudeRate_t QSerial::Baude19200 = B19200;

QSerial::QSerial() :
    fd(-1)
{
}

QSerial::~QSerial()
{
    close();
}

void QSerial::close()
{
    if (isOpen()) {
        ::close(fd);
        fd = -1;
    }
}

bool QSerial::isOpen()
{
    return (fd >= 0);
}

bool QSerial::isLine(const char *buf, ssize_t size)
{
    // empty line is not line
    if (size <= 1)
        return false;

    if (buf[size - 1] == '\n' || buf[size - 1] == '\r')
        return true;

    return false;
}

/**
 * Reads data from serial port.
 * @param buf   Buffer to store readed data.
 * @param count Maximal amount of bytes to read.
 * @return      Number of bytes succesfully readed, or gefative number
 *      (error no.) on error.
 */
ssize_t QSerial::read(char *buf, ssize_t count)
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
                size_ = ::read(fd, buf, count);
                if (size_ < 0)
                        return -1;
                size += size_;
                count -= size_;
                buf += size_;
                if (isLine(buf_, size))
                        return size;
        } while (count > 0);

        errno = ERANGE;
        return -1;
}


/**
 * Open serial port and set parameters as defined for SDP power source.
 * @param port File name of serial port.
 * @return      File descriptor on success, negative number (err no.) on error.
 */
bool QSerial::open(const char *port, BaudeRate_t bauderate, int timeout = 0)
{
        struct termios tio;

        fd = ::open(port, O_RDWR | O_NONBLOCK);
        if (fd < 0)
                return -1;

        memset(&tio, 0, sizeof(tio));
        tio.c_cflag = CS8 | CREAD | CLOCAL;
        tio.c_cc[VMIN] = 1;
        tio.c_cc[VTIME] = 5;
        cfsetispeed(&tio, bauderate);
        cfsetospeed(&tio, bauderate);

        if (tcsetattr(fd, TCSANOW, &tio) < 0) {
                int e = errno;
                ::close(fd);
                errno = e;

                return -1;
        }

        return fd;
}

bool QSerial::open(const QString &port, BaudeRate_t bauderate, int timeout)
{
    const char *port_str;

    port_str = port.toLocal8Bit().constData();
    return open(port_str, bauderate, timeout);
}

QSerial QSerial::operator<<(const char *str)
{
    if (::write(fd, str, strlen(str)) < 0)
        ; // TODO
}

QSerial QSerial::operator<<(const QString &str)
{
}

QSerial QSerial::operator<<(const int &i)
{
}

QSerial QSerial::operator>>(QString &str)
{
}

