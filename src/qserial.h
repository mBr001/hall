#ifndef QSERAL_H
#define QSERAL_H

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
    bool open(const QString &port, BaudeRate_t bauderate, long timeoutOffs,
              long timeoutPerChar);
    
    QSerial operator<<(const char *str);
    QSerial operator<<(const QString &str);
    QSerial operator<<(const int &i);
    QSerial operator>>(QString &str);
private:
    int fd;
    long timeoutOffs, timeoutPerChar;

    bool isLine(const char *buf, ssize_t size);
    ssize_t read(char *buf, ssize_t count);
};

#endif // PS6220HACK_H
