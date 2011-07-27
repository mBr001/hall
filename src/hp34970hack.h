#ifndef HP34970HACK_H
#define HP34970HACK_H

#include "qserial.h"
#include <qlist.h>

class HP34970hack : protected QSerial
{
public:
    typedef enum {

    } Sense_t;

    HP34970hack();
    ~HP34970hack();
    void close();
    void init();
    bool open(QString port);
    void routeChannels(QList<int> openChannels, int offs);
    void setScan(QList<int> channels);
    void setSense(int channel, Sense_t sense);
    void setup();
    QString readCmd();

protected:
    void sendCmd(QString cmd, long timeout = 0);
    QString sendQuery(QString cmd, long timeout = 0);

private:

};

#endif // HP34970HACK_H
