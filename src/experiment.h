#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <msdp2xxx.h>
#include "config.h"
#include "hp34970hack.h"
#include "powpolswitch.h"
#include "ps6220hack.h"
#include "qcsvfile.h"

class Experiment
{
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
        /** csvColEnd is number of columns we have not a real column. */
        csvColEnd,
    };

    Experiment();
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

    // TODO: až to bude přeportované přepnout na protexted
public:
    /** File to save measured data. */
    QCSVFile csvFile;

private:
    Config config;
};

#endif // EXPERIMENT_H
