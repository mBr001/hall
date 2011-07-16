#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/select.h>
#include <unistd.h>
#include <QtCore>
#include <stdexcept>

#include "qserial.h"


const QSerial::BaudeRate_t QSerial::Baude9600 = B9600;
const QSerial::BaudeRate_t QSerial::Baude19200 = B19200;

void stdError(std::string str)
{
    int err = errno;

    std::string err_str(strerror(err));

    throw new std::runtime_error(str + err_str);
}

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
    // empty line is not real line
    if (size <= 1)
        return false;

    if (buf[size - 1] == '\n' || buf[size - 1] == '\r')
        return true;

    return false;
}

/**
 * Open serial port and set parameters as defined for SDP power source.
 * @param port File name of serial port.
 * @param bauderate bauderate of serial port.
 * @param timeout Time reserved for reading [usec].
 * @param timeoutPerChar Increase reading time timeout per recieved character [us].
 * @return       File descriptor on success, negative number (err no.) on error.
 */
bool QSerial::open(const char *port, BaudeRate_t bauderate, long timeout,
                   long timeoutPerChar)
{
    struct termios tio;

    fd = ::open(port, O_RDWR | O_NONBLOCK);
    if (fd < 0)
         return false;

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

        return false;
    }

    this->timeoutOffs = timeout;
    this->timeoutPerChar = timeoutPerChar;
    return fd;
}

bool QSerial::open(QString port, BaudeRate_t bauderate, long timeout,
                   long timeoutPerChar)
{
    const char *port_str;

    port_str = qPrintable(port);
    return open(port_str, bauderate, timeout, timeoutPerChar);
}

QString QSerial::readLine(ssize_t count, long timeout = 0)
{
    char buf[count];
    ssize_t len;

    len = readLine(buf, sizeof(buf), timeout);
    if (len <= 0)
        stdError("QSerial::readLine read failed.");
    buf[len] = 0;

    return buf;
}

/**
 * Reads data from serial port.
 * @param buf   Buffer to store readed data.
 * @param count Maximal amount of bytes to read.
 * @return      Number of bytes succesfully readed, or gefative number
 *      (error no.) on error.
 */
ssize_t QSerial::readLine(char *buf, ssize_t count, long timeout)
{
    const char *buf_ = buf;
    fd_set readfds;
    int ret;
    ssize_t size = 0;
    struct timeval timeout_s;

    timeout = timeout ? timeout : timeoutOffs;
    timeout_s.tv_sec = timeout / 1000000l;
    timeout_s.tv_usec = timeout % 1000000l;

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    do {
        ssize_t size_;

        ret = select(fd + 1, &readfds, NULL, NULL, &timeout_s);
        if (ret <= 0) {
                return -1;
        }
        size_ = ::read(fd, buf, count);
        if (size_ < 0)
                return size;
        timeout_s.tv_usec += timeoutPerChar * size_;
        timeout_s.tv_sec += timeout_s.tv_usec / 1000000l;
        timeout_s.tv_usec %= 1000000l;
        size += size_;
        count -= size_;
        buf += size_;
        if (isLine(buf_, size))
                return size;
    } while (count > 0);

    errno = ERANGE;
    return -1;
}

void QSerial::write(const char *str)
{
    if (::write(fd, str, strlen(str)) < 0)
        stdError("Failed to write to serial port");
}

void QSerial::write(QString str)
{
    QByteArray bytes;

    bytes = str.toLocal8Bit();
    if (::write(fd, bytes.constData(), bytes.length()) < 0)
        stdError("Failed to write to serial port");
}

void QSerial::write(int i)
{
    QVariant v(i);
    QByteArray b;

    b = v.toString().toLocal8Bit();
    if (::write(fd, b.constData(), b.length()) < 0)
        stdError("Failed to write to serial port");
}
