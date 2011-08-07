#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <QObject>
#include <msdp2xxx.h>

#include "config.h"
//#include "hp34970hack.h"
//#include "powpolswitch.h"
//#include "ps6220hack.h"
#include "qcsvfile.h"

class Experiment : public QObject
{
    Q_OBJECT
public:
    /** Indexes of columns in CSV file with data from experiment. */
    enum {
        csvColTime = 0,
        csvColHallProbeI,
        csvColHallProbeU,
        csvColSampleI,
        csvColSampleUacF,
        csvColSampleUacB,
        csvColSampleUbdF,
        csvColSampleUbdB,
        csvColSampleUcdF,
        csvColSampleUcdB,
        csvColSampleUdaF,
        csvColSampleUdaB,
        csvColEmpty,
        csvColHallProbeB,
        csvColSampleUac,
        csvColSampleUbd,
        csvColSampleUcd,
        csvColSampleUda,
        /** csvColEnd equeals to a number of columns, it is not a column at all. */
        csvColEnd,
    };

    explicit Experiment(QObject *parent = 0);

    void close();
    void open();

    void measurementStartIntervall();
    void measurementStartSingle();
    void measurementAbort();

    double coilI();
    void setCoilI(double val);

    double sampleI();
    void setSampleI(double val);

    void setBCoefficients(double A, double B);
    double getBCoefficientA();
    double getBCoefficientB();

    QString HP34970Port();
    void setHP34970Port(QString port);
    QString PS6220Port();
    void setPS6220Port(QString port);
    QString MSDPPort();
    void setMSDPPort(QString port);

    /** Return time data for last measurement as string. */
    QString strDataTime();

    // TODO: tohle budou interní funkce, převážně přijdou odstranit
    void _csvFileWrite();
    void _csvFileGetTime();

    // TODO: až to bude přeportované přepnout na protected
public:
    /** File to save measured data. */
    QCSVFile csvFile;

signals:
    void measurementComleted();

public slots:

private:
    Config config;
    QString _strDataTime_;
};

#endif // EXPERIMENT_H
