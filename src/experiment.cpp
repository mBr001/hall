#include "error.h"
#include "experiment.h"

Experiment::Experiment(QObject *parent) :
    QObject(parent)
{
}

void Experiment::close()
{
    csvFile.close();
}

void Experiment::_csvFileWrite()
{
    csvFile.write();
    emit measurementComleted();
}

void Experiment::open()
{
    csvFile.setFileName(config.dataFileName());
    if (!csvFile.open()) {
        throw new Error("Failed to open data file",
                        csvFile.errorString());
    }

    csvFile.resize(csvColEnd);
    csvFile[csvColTime] = "Time";
    csvFile[csvColHallProbeI] = "Hall proble";
    csvFile[csvColHallProbeU] = "Hall proble";
    csvFile[csvColHallProbeB] = "Hall proble";
    csvFile[csvColSampleI] = "sample";
    csvFile[csvColSampleUacF] = "sample";
    csvFile[csvColSampleUacB] = "sample";
    csvFile[csvColSampleUbdF] = "sample";
    csvFile[csvColSampleUbdB] = "sample";
    csvFile[csvColSampleUcdF] = "sample";
    csvFile[csvColSampleUcdB] = "sample";
    csvFile[csvColSampleUdaF] = "sample";
    csvFile[csvColSampleUdaB] = "sample";
    if (!csvFile.write()) {
        csvFile.close();
        throw new Error("Failed to write header into data file",
                        csvFile.errorString());
    }

    csvFile[csvColTime] = "(UTC)";
    csvFile[csvColHallProbeI] = "I [A]";
    csvFile[csvColHallProbeU] = "U [V]";
    csvFile[csvColHallProbeB] = "B [T]";
    csvFile[csvColSampleI] = "I [A]";
    csvFile[csvColSampleUacF] = "Uac/+- [V]";
    csvFile[csvColSampleUacB] = "Uac/-+ [V]";
    csvFile[csvColSampleUbdF] = "Ubd/+- [V]";
    csvFile[csvColSampleUbdB] = "Ubd/-+ [V]";
    csvFile[csvColSampleUcdF] = "Ucd/+- [V]";
    csvFile[csvColSampleUcdB] = "Ucd/-+ [V]";
    csvFile[csvColSampleUdaF] = "Uda/+- [V]";
    csvFile[csvColSampleUdaB] = "Uda/-+ [V]";
    csvFile.write();
    if (!csvFile.write()) {
        csvFile.close();
        throw new Error("Failed to write header into data file",
                        csvFile.errorString());
    }
}

QString Experiment::strDataTime()
{
    return _strDataTime_;
}

void Experiment::_csvFileGetTime()
{
    _strDataTime_ = csvFile.setAt(csvColTime, QDateTime::currentDateTimeUtc());
}
