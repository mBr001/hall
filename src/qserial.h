#ifndef QSERAL_H
#define QSERAL_H

#include <QtCore>


class QSerial
{
public:
    typedef int BaudeRate_t;

    static const BaudeRate_t Baude9600;
    static const BaudeRate_t Baude19200;

    QSerial();
    ~QSerial();
    void close();
    bool isOpen();
    /** List all available serial ports. */
    static QStringList list();
    /**
     * Open serial port and set parameters as defined for SDP power source.
     * @param serial port name.
     * @param bauderate bauderate of serial port.
     * @param timeout Time reserved for reading [usec].
     * @param timeoutPerChar Increase reading time timeout per recieved character [us].
     * @return       File descriptor on success, negative number (err no.) on error.
     */
    bool open(const char *port, BaudeRate_t bauderate, long timeoutOffs,
              long timeoutPerChar);
    /**
     * Open serial port and set parameters as defined for SDP power source.
     * @param serial port name.
     * @param bauderate bauderate of serial port.
     * @param timeout Time reserved for reading [usec].
     * @param timeoutPerChar Increase reading time timeout per recieved character [us].
     * @return       File descriptor on success, negative number (err no.) on error.
     */
    bool open(const QString &port, BaudeRate_t bauderate, long timeoutOffs,
              long timeoutPerChar);
    
    /** Write null terminated string into serial line. */
    bool write(const char *str);
    /** Write UTF-8 string into serial line. */
    bool write(const QString &str);
    bool readLine(QString &str, ssize_t count);
    bool readLine(QString &str, ssize_t maxSize, long timeout);
private:
    int fd;
    long timeoutOffs, timeoutPerChar;

    bool isLine(const char *buf, ssize_t size);
    ssize_t readLine(char *buf, ssize_t count, long timeout);
};

#endif // QSERAL_H
