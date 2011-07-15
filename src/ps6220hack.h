#ifndef PS6220HACK_H
#define PS6220HACK_H

class PS6220Hack
{
public:
    PS6220Hack();
    void close();
    double current();
    bool open(const char *);
    bool output();
    void setCurrent(double current);
    void setOutput(bool out);

private:
    int fd;

    bool isline(const char *buf, ssize_t size);
    int serial_open(const char* fname);
    ssize_t serial_read(char *buf, ssize_t count);
};

#endif // PS6220HACK_H
