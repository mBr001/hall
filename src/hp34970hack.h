#ifndef HP34970HACK_H
#define HP34970HACK_H

#include "qserial.h"
#include <qlist.h>
#include <qstringlist.h>

class HP34970hack : protected QSerial
{
public:
    typedef const char *Sense_t;

    static Sense_t SenseVolt;
    static Sense_t SenseRes;

    HP34970hack();
    ~HP34970hack();
    void close();
    void init();
    bool open(QString port);
    QStringList read();
    void routeChannels(QList<int> closeChannels, int offs);
    void setScan(QList<int> channels);
    void setSense(QList<int> channels, Sense_t sense);
    void setup();

protected:
    void sendCmd(QString cmd, long timeout = 0);
    QString sendQuery(QString cmd, long timeout = 0);

private:

};

#endif // HP34970HACK_H
