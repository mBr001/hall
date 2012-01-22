#ifndef HALLDATA_H
#define HALLDATA_H

#include <QObject>
#include <QVector>

class HallData : public QObject
{
    Q_OBJECT
public:
    explicit HallData(QObject *parent = 0);

    class EvaluatedData {
    public:
        EvaluatedData();

        double B;
        double carrierConcentration;
        double driftSpeed;
        double R;
        double Rhall;
        double Rspec;
        double Uhall;
    };

    class MeasuredData {
    public:
        MeasuredData();

        double hallProbeI, hallProbeU;
        double sampleI;
        double sampleUac, sampleUacRev;
        double sampleUbd, sampleUbdRev;
        double sampleUcd, sampleUcdRev;
        double sampleUda, sampleUdaRev;
    };

private:
    QVector<double> _hallProbeI_,
        _hallProbeU_,
        _sampleI_,
        _sampleUac_, _sampleUacRev_,
        _sampleUbd_, _sampleUbdRev_,
        _sampleUcd_, _sampleUcdRev_,
        _sampleUda_, _sampleUdaRev_;

    QVector<double> _B_,
        _carrierConcentration_,
        _driftSpeed_,
        _R_,
        _Rhall_,
        _Rspec_,
        _Uhall_;

signals:
    void measurementAcquired(const MeasuredData &measuredData,
                          EvaluatedData &evaluatedData);
    void measurementAdded(const MeasuredData &measuredData,
                          const EvaluatedData &evaluatedData);

public slots:
    void addMeasurement(const MeasuredData &measurementData);

};

#endif // HALLDATA_H
