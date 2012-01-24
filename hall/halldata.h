#ifndef HALLDATA_H
#define HALLDATA_H

//#include <boost/units/systems/si/io.hpp>
#include <QDateEdit>
#include <QObject>
#include <QVector>

class HallData : public QObject
{
    Q_OBJECT
public:
    explicit HallData(QObject *parent = 0);

    class EvaluatedData {
    public:
        explicit EvaluatedData();

        double B;
        double driftSpeed;
        double errAsymetry, errShottky;
        double R;
        double Rhall;
        double Rspec;
        double Uhall;
    };

    class MeasuredData {
    public:
        explicit MeasuredData();

        double hallProbeU;
        double sampleI;
        double sampleUac, sampleUacRev;
        double sampleUbd, sampleUbdRev;
        double sampleUcd, sampleUcdRev;
        double sampleUda, sampleUdaRev;
        QDateTime time;
    };

    class SummaryData {
    public:
        explicit SummaryData();

        double carrierc, driftSpeed, R, RSpec;
    };

private:
    QVector<QDateTime> _time_;
    QVector<double> _hallProbeU_,
        _sampleI_,
        _sampleUac_, _sampleUacRev_,
        _sampleUbd_, _sampleUbdRev_,
        _sampleUcd_, _sampleUcdRev_,
        _sampleUda_, _sampleUdaRev_;
    QVector<double> _B_,
        _driftSpeed_,
        _R_,
        _Rhall_,
        _Rspec_,
        _Uhall_;

signals:
    void measurementAcquired(
        const HallData::MeasuredData &measuredData,
        HallData::EvaluatedData &evaluatedData);
    void measurementAdded(
        const HallData::MeasuredData &measuredData,
        const HallData::EvaluatedData &evaluatedData);

public slots:
    void addMeasurement(const MeasuredData &measurementData);
    void clear();

    const QVector<double> &B() const;
    const QVector<double> &driftSpeed() const;
    const QVector<double> &R() const;
    const QVector<double> &RSpec() const;
    const QVector<double> &Uhall() const;

};

#endif // HALLDATA_H
