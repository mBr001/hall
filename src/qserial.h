#ifndef PS6220HACK_H
#define PS6220HACK_H

class QSerial
{
public:

    typedef int BaudeRate_t;

    static const BaudeRate_t Baude9200;
    static const BaudeRate_t Baude19200;


    QSerial();
    ~QSerial();
    void close();
    bool isOpen();
    bool open(const char *port, BaudeRate_t bauderate, int timeout);
    bool open(const QString &port, BaudeRate_t bauderate, int timeout);
    
    QSerial operator<<(const char *str);
    QSerial operator<<(const QString &str);
    QSerial operator<<(const int &i);
    QSerial operator>>(QString &str);
private:
    int fd;

    bool isLine(const char *buf, ssize_t size);
    ssize_t read(char *buf, ssize_t count);
};

#endif // PS6220HACK_H
