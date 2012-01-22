#include "halldata.h"

#include <math.h>

HallData::MeasuredData::MeasuredData() :
    hallProbeI(NAN), hallProbeU(NAN),
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

    _hallProbeI_.append(measuredData.hallProbeI);
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
