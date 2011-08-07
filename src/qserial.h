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
    bool open(const char *port, BaudeRate_t bauderate, long timeoutOffs,
              long timeoutPerChar);
    bool open(QString port, BaudeRate_t bauderate, long timeoutOffs,
              long timeoutPerChar);
    
    void write(const char *str);
    void write(QString str);
    void write(int i);
    QString readLine(ssize_t count);
    QString readLine(ssize_t count, long timeout);
private:
    int fd;
    long timeoutOffs, timeoutPerChar;

    bool isLine(const char *buf, ssize_t size);
    ssize_t readLine(char *buf, ssize_t count, long timeout);
};

#endif // PS6220HACK_H
