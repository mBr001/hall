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
protected:
    void cmd(QString cmd);
    QString query(const QString &cmd);

private:

};

#endif // HP34970HACK_H
