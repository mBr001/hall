#ifndef PS6220HACK_H
#define PS6220HACK_H

class HP34970hack
{
public:
    HP34970hack();
    ~HP34970hack();
    void close();
    bool open(const char *);
    void setChannel(int channel, bool open);
    void setSense(int channel, Sense_t sense);
    void setup();



private:
    int fd;

    bool isline(const char *buf, ssize_t size);
    int serial_open(const char* fname);
    ssize_t serial_read(char *buf, ssize_t count);
};

#endif // PS6220HACK_H
