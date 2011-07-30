#ifndef HP34970HACK_H
#define HP34970HACK_H

#include "qserial.h"
#include <qlist.h>
#include <qstringlist.h>

class HP34970hack : protected QSerial
{
public:
    typedef const char *Sense_t;
    typedef QList<int> Channels_t;

    static Sense_t SenseVolt;
    static Sense_t SenseRes;

    HP34970hack();
    ~HP34970hack();
    void close();
    void init();
    bool open(QString port);
    QStringList read();
    void setRoute(Channels_t closeChannels, int offs);
    void setScan(Channels_t channels);
    void setSense(Sense_t sense, Channels_t channels);
    void setup();

protected:
    void sendCmd(QString cmd, long timeout = 0);
    void sendCmd(QString cmd, const Channels_t &channels, long timeout = 0);
    QString sendQuery(QString cmd, long timeout = 0);
    QString sendQuery(QString cmd, const Channels_t &channels, long timeout = 0);

private:

};

#endif // HP34970HACK_H
