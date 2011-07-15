#ifndef HP34970HACK_H
#define HP34970HACK_H

#include "qserial.h"

class HP34970hack : protected QSerial
{
public:
    typedef enum {

    } Sense_t;

    HP34970hack();
    ~HP34970hack();
    void close();
    bool open(const QString &port);
    void setChannel(int channel, bool open);
    void setSense(int channel, Sense_t sense);
    void setup();
    QString readCmd();

protected:
    void sendCmd(const QString cmd);
    void sendCmd(const QString cmd, long timeout);
    QString query(const QString cmd);
    QString query(const QString cmd, long timeout);

private:

};

#endif // HP34970HACK_H
