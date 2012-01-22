#include "halldata.h"

#include <math.h>

HallData::MeasuredData::MeasuredData() :
    hallProbeU(NAN),
    sampleI(NAN),
    sampleUac(NAN), sampleUacRev(NAN),
    sampleUbd(NAN), sampleUbdRev(NAN),
    sampleUcd(NAN), sampleUcdRev(NAN),
    sampleUda(NAN), sampleUdaRev(NAN)
{
}

HallData::EvaluatedData::EvaluatedData() :
    B(NAN),
    carrierConcentration(NAN),
    driftSpeed(NAN),
    errAsymetry(NAN), errShottky(NAN),
    R(NAN),
    Rhall(NAN),
    Rspec(NAN),
    Uhall(NAN)
{
}

HallData::HallData(QObject *parent) :
    QObject(parent)
{

}

void HallData::addMeasurement(const MeasuredData &measuredData)
{
    EvaluatedData evaluatedData;

    emit measurementAcquired(measuredData, evaluatedData);

    _hallProbeU_.append(measuredData.hallProbeU);
    _sampleI_.append(measuredData.sampleI);
    _sampleUac_.append(measuredData.sampleUac);
    _sampleUacRev_.append(measuredData.sampleUacRev);
    _sampleUbd_.append(measuredData.sampleUbd);
    _sampleUbdRev_.append(measuredData.sampleUbdRev);
    _sampleUcd_.append(measuredData.sampleUcd);
    _sampleUcdRev_.append(measuredData.sampleUcdRev);
    _sampleUda_.append(measuredData.sampleUda);
    _sampleUdaRev_.append(measuredData.sampleUdaRev);

    _B_.append(evaluatedData.B);
    _carrierConcentration_.append(evaluatedData.carrierConcentration);
    _driftSpeed_.append(evaluatedData.driftSpeed);
    _R_.append(evaluatedData.R);
    _Rhall_.append(evaluatedData.Rhall);
    _Rspec_.append(evaluatedData.Rspec);
    _Uhall_.append(evaluatedData.Uhall);

    emit measurementAdded(measuredData, evaluatedData);
}

const QVector<double> &HallData::B() const
{
    return _B_;
}

void HallData::clear()
{
    _time_.clear();

    _hallProbeU_.clear();
    _sampleI_.clear();
    _sampleUac_.clear();
    _sampleUacRev_.clear();
    _sampleUbd_.clear();
    _sampleUbdRev_.clear();
    _sampleUcd_.clear();
    _sampleUcdRev_.clear();
    _sampleUda_.clear();
    _sampleUdaRev_.clear();

    _B_.clear();
    _carrierConcentration_.clear();
    _driftSpeed_.clear();
    _R_.clear();
    _Rhall_.clear();
    _Rspec_.clear();
    _Uhall_.clear();
}

const QVector<double> &HallData::R() const
{
    return _R_;
}

const QVector<double> &HallData::Uhall() const
{
    return _Uhall_;
}
