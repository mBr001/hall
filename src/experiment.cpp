#include <errno.h>
#include <math.h>

#include "error.h"
#include "experiment.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#endif

const int Experiment::_34901A = 100;
const int Experiment::_34901A_sample_cd = _34901A + 1;
const int Experiment::_34901A_sample_da = _34901A + 2;
const int Experiment::_34901A_sample_bd = _34901A + 3;
const int Experiment::_34901A_sample_ac = _34901A + 4;
const int Experiment::_34901A_hall_probe = _34901A + 14;

const int Experiment::_34903A = 200;
const int Experiment::_34903A_sample_a_pwr_m = _34903A + 1;
const int Experiment::_34903A_sample_b_pwr_p = _34903A + 2;
const int Experiment::_34903A_sample_c_pwr_sw1 = _34903A + 3;
const int Experiment::_34903A_sample_d_pwr_m = _34903A + 4;
const int Experiment::_34903A_pwr_sw1_pwr_m = _34903A + 5;
const int Experiment::_34903A_pwr_sw1_pwr_p = _34903A + 6;
const int Experiment::_34903A_hall_probe_1_pwr_m = _34903A + 9;
const int Experiment::_34903A_hall_probe_2_pwr_p = _34903A + 10;

const double Experiment::hallProbeI = 0.001;

const Experiment::Step_t Experiment::stepsAll[] = {
    {   stepAbort, 0,    },
};

const Experiment::Step_t Experiment::stepsMeasure[] = {
    {   stepOpenAllRoutes, 10, },
    {   stepGetTime, 0 },

    {   stepSamplePower_pm, 10 },
    {   stepSamplePower_bd, 10 },
    {   stepSampleMeasPrepare_ac, 10 },
    {   stepSampleMeas_ac, 10 },
    {   stepSamplePower_ca, 10 },
    {   stepSampleMeasPrepare_bd, 10 },
    {   stepSampleMeas_bd, 10 },

    {   stepSamplePower_ba, 10 },
    {   stepSampleMeasPrepare_cd, 10 },
    {   stepSampleMeas_cd, 10 },
    {   stepSamplePower_bc, 10 },
    {   stepSampleMeasPrepare_da, 10 },
    {   stepSampleMeas_da, 10 },

    {   stepSamplePower_mp, 10 },
    {   stepSamplePower_bd, 10 },
    {   stepSampleMeasPrepare_ac, 10 },
    {   stepSampleMeas_acRev, 10 },
    {   stepSamplePower_ca, 10 },
    {   stepSampleMeasPrepare_bd, 10 },
    {   stepSampleMeas_bdRev, 10 },

    {   stepSamplePower_ba, 10 },
    {   stepSampleMeasPrepare_cd, 10 },
    {   stepSampleMeas_cdRev, 10 },
    {   stepSamplePower_bc, 10 },
    {   stepSampleMeasPrepare_da, 10 },
    {   stepSampleMeas_daRev, 10 },

    {   stepMeasHallProbePrepare, 10 },
    {   stepMeasHallProbe, 0 },
    {   stepFinish, 0 },
    {   stepAbort, 0 },
};

Experiment::Steps_t::Steps_t(const Step_t *begin, const Step_t *end)
{
    reserve(end - begin);
    for (; begin < end; ++begin) {
        append(*begin);
    }
}

Experiment::Experiment(QObject *parent) :
    QObject(parent),
    coilTimer(this),
    _coilWantI_(0),
    measTimer(this),
    _measuring_(false),
    _sweeping_(false),
    B1(NAN),
    B2(NAN),
    B3(NAN),
    _dataHallU0_(NAN),
    _sampleI_(0)
{
    coilTimer.setObjectName("coilTimer");
    coilTimer.setInterval(currentDwell);
    coilTimer.setSingleShot(false);

    measTimer.setObjectName("measTimer");
    measTimer.setSingleShot(true);

    QMetaObject::connectSlotsByName(this);
}

void Experiment::close()
{
    coilTimer.stop();
    measTimer.stop();
    csvFile.close();
    hp34970Hack.setRoute(HP34970Hack::Channels_t(), _34903A);
    hp34970Hack.close();
    ps622Hack.setOutput(false);
    // FIXME: hack, je to kvůli tomu aby si pamatoval posladní nastavené U pro vzorek
    ps622Hack.setCurrent(_sampleI_);
    ps622Hack.close();
    pwrPolSwitch.close();
    sdp_close(&sdp);
}

double Experiment::coefficientB1()
{
    return B1;
}

double Experiment::coefficientB2()
{
    return B2;
}

double Experiment::coefficientB3()
{
    return B3;
}

double Experiment::coilI()
{
    return _coilWantI_;
}

double Experiment::coilMaxI()
{
    return _coilMaxI_;
}

double Experiment::computeB(double U)
{
    double B(B1 + sqrt(B2 + B3 * fabs(U) / _sampleI_));
    // alternativní vzorec a čísla
    // U /= I; B = U(A+sqrt(U)*(B+C*sqrt(U)))-D;
    // A=5.97622E-4 B=1.591394E-6 C=-9.24701E-11 D=-0.015

    return U > 0 ? B : -B;
}

void Experiment::measurementStop()
{
    if (_measuring_) {
        _measuring_ = false;
        measTimer.stop();
        emit measurementCompleted();
    }

    // Set up measurement to measure B at coil by default
    // used for periodical B measurement when idle
    ps622Hack.setOutput(false);

    HP34970Hack::Channels_t closeChannels;
    closeChannels.append(_34903A_hall_probe_1_pwr_m);
    closeChannels.append(_34903A_hall_probe_2_pwr_p);
    hp34970Hack.setRoute(closeChannels, _34903A);

    HP34970Hack::Channels_t scan;
    scan.append(Experiment::_34901A_hall_probe);
    hp34970Hack.setScan(scan);

    ps622Hack.setCurrent(hallProbeI);
    ps622Hack.setOutput(true);
}

bool Experiment::isMeasuring()
{
    return _measuring_;
}

void Experiment::measure(bool single)
{
    if (single) {
        stepsRunning = Steps_t(
                    stepsMeasure,
                    stepsMeasure + ARRAY_SIZE(stepsMeasure));
        stepCurrent = stepsRunning.begin();
    } else {
        throw new Error("Not implemented.");
    }

    _measuring_ = true;
    measTimer.start(0);
}

void Experiment::on_coilTimer_timeout()
{
    sdp_lcd_info_t lcd_info;

    if (sdp_get_lcd_info(&sdp, &lcd_info) < 0) {
        throw new std::runtime_error(
                "on_currentTimer_timeout - sdp_get_lcd_info");
        return;
    }

    emit coilIMeasured(lcd_info.read_A);
    emit coilUMeasured(lcd_info.read_V);

    // if no measurement in progress we measure B themselv (at proper conditions)
    if (!_measuring_) {
        double U(readSingle());
        emit coilBMeasured(computeB(U));
    }

    // update coil current value
    if (!_sweeping_)
        return;

    /** Curent through coil. */
    double procI;
    /** Coil power state, on/off. */
    bool procCoilPower, wantCoilPower;
    /** Coil power switch state direct/cross */
    PwrPolSwitch::state_t procCoilSwitchState, wantCoilSwitchState;

    /* Get all values necesary for process decisions. */
    // wanted values
    if (_coilWantI_ < 0)
        wantCoilSwitchState = PwrPolSwitch::cross;
    else
        wantCoilSwitchState = PwrPolSwitch::direct;

    wantCoilPower = (_coilWantI_ != 0);

    // process values
    procCoilSwitchState = pwrPolSwitch.polarity();

    procCoilPower = lcd_info.output;
    procI = lcd_info.set_A;
    if (procCoilSwitchState == PwrPolSwitch::cross)
        procI = -procI;

    /* Make process decision. */
    // Need switch polarity?
    if (procCoilSwitchState != wantCoilSwitchState) {
        // Is polarity switch posible? (power is off)
        if (!procCoilPower) {
            pwrPolSwitch.setPolarity(wantCoilSwitchState);
            return;
        }

        // Is posible power-off in order to swich polarity?
        if (fabs(procI) < currentSlope) {
            sdp_set_output(&sdp, 0);
            return;
        }

        // we must set current near to zero before polarity switching
    }

    // Target reach, finish job
    if (fabs(procI - _coilWantI_) < currentSlope) {
        if (!wantCoilPower && fabs(procI) <= currentSlope && procCoilPower) {
            if (sdp_set_output(&sdp, 0) < 0)
                throw new Error("timer - sdp_set_output");
        }

        emit sweepingCompleted();
        _sweeping_ = false;
        return;
    }

    // want current but power is off -> set power on at current 0.0 A
    if (procCoilPower != wantCoilPower && wantCoilPower) {
        sdp_set_curr(&sdp, 0.0);
        sdp_set_output(&sdp, 1);
        return;
    }

    // power is on, but current neet to be adjusted, do one step
    if (procI > _coilWantI_)
        procI -= currentSlope;
    else
        procI += currentSlope;

    sdp_set_curr(&sdp, fabs(procI));
}

void Experiment::on_measTimer_timeout()
{
    if (stepCurrent != stepsRunning.end()) {
        stepCurrent->func(this);
        if (stepCurrent != stepsRunning.end()) {
            measTimer.start(stepCurrent->delay);
            ++stepCurrent;
            return;
        }
    }
    measurementStop();
}

void Experiment::open()
{
    QString port;
    int err;

    port = config.msdpPort();
    err = sdp_open(&sdp, port.toLocal8Bit().constData(), SDP_DEV_ADDR_MIN);
    if (err < 0) {
        throw new Error("Manson SDP power supply operation failed",
                        QString::fromLocal8Bit(sdp_strerror(err)));
    }

    try {
        /* Set value limit in current input spin box. */
        sdp_va_t limits;
        err = sdp_get_va_maximums(&sdp, &limits);
        if (err < 0) {
            throw new Error("Manson SDP power supply operation failed",
                            QString::fromLocal8Bit(sdp_strerror(err)));
        }
        _coilMaxI_ = limits.curr;

        /* Set actual current value as wanted value, avoiding unwanted hickups. */
        sdp_va_data_t va_data;
        err = sdp_get_va_data(&sdp, &va_data);
        if (err < 0) {
            throw new Error("Manson SDP power supply operation failed",
                            QString::fromLocal8Bit(sdp_strerror(err)));
        }
        _coilWantI_ = va_data.curr;

        err = sdp_set_curr(&sdp, va_data.curr);
        if (err < 0) {
            throw new Error("Manson SDP power supply operation failed",
                            QString::fromLocal8Bit(sdp_strerror(err)));
        }

        /* Set voltage to maximum, we drive only current. */
        err = sdp_set_volt_limit(&sdp, limits.volt);
        if (err < 0) {
            throw new Error("Manson SDP power supply operation failed",
                            QString::fromLocal8Bit(sdp_strerror(err)));
        }

        err = sdp_set_volt(&sdp, limits.volt);
        if (err < 0) {
            throw new Error("Manson SDP power supply operation failed",
                            QString::fromLocal8Bit(sdp_strerror(err)));
        }

        sdp_lcd_info_t lcd_info;
        err = sdp_get_lcd_info(&sdp, &lcd_info);
        if (err < 0) {
            throw new Error("Manson SDP power supply operation failed",
                            QString::fromLocal8Bit(sdp_strerror(err)));
        }
        if (!lcd_info.output)
            _coilWantI_ = 0;

        /* Data file preparation. */
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
        csvFile[csvColSampleResistivity] = "sample";
        csvFile[csvColSampleResSpec] = "sample";
        csvFile[csvColSampleRHall] = "sample";
        csvFile[csvColSampleDrift] = "sample";
        if (!csvFile.write()) {
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
        csvFile[csvColSampleResistivity] = "R [ohm]";
        csvFile[csvColSampleResSpec] = "Rspec [ohm*m]";
        csvFile[csvColSampleRHall] = "Rhall [m^3*C^-1]";
        csvFile[csvColSampleDrift] = "drift [m^2*V^-1*s^-1]";
        csvFile.write();
        if (!csvFile.write()) {
            throw new Error("Failed to write header into data file",
                            csvFile.errorString());
        }

        // Open polarity switch device
        port = config.polSwitchPort();
        if (!pwrPolSwitch.open(port.toLocal8Bit().constData())) {
            err = errno;
            throw new Error("Failed to open coil polarity switch port",
                            QString::fromLocal8Bit(strerror(err)));
        }

        if (pwrPolSwitch.polarity() == PwrPolSwitch::cross) {
            _coilWantI_ = -_coilWantI_;
        }

        // Open sample power source
        port = config.ps6220Port();
        if (!ps622Hack.open(port.toLocal8Bit().constData()))
        {
            err = errno;
            throw new Error("Failed to open sample power supply (Keithaly 6220)",
                            QString::fromLocal8Bit(strerror(err)));
        }
        _sampleI_ = ps622Hack.current();

        // Open and setup HP34970 device
        if (!hp34970Hack.open(config.hp34970Port())) {
            err = errno;
            throw new Error("Failed to open HP34970 device",
                            QString::fromLocal8Bit(strerror(err)));
        }
        HP34970Hack::Channels_t channels;

        channels << 101 << 102 << 103 << 104 << 114;
        hp34970Hack.setSense(HP34970Hack::SenseVolt, channels);
        // set up measurement of B
        measurementStop();

        coilTimer.start();
    }
    catch (Error e)
    {
        csvFile.close();
        sdp_close(&sdp);
        pwrPolSwitch.close();
        ps622Hack.close();

        throw;
    }
}

double Experiment::readSingle()
{
    QStringList data(hp34970Hack.read());
    if (data.size() != 1) {
        throw new std::runtime_error("Could not get B data.");
    }

    bool ok;
    double val(QVariant(data[0]).toDouble(&ok));
    if (!ok)
        return NAN;
    return val;
}

double Experiment::sampleI()
{
    return ps622Hack.current();
}

double Experiment::sampleThickness()
{
    return _sampleThickness_;
}

void Experiment::setCoefficients(double B1, double B2, double B3)
{
    this->B1 = B1;
    this->B2 = B2;
    this->B3 = B3;
}

void Experiment::setCoilI(double value)
{
    _coilWantI_ = value;
    _sweeping_ = true;
}

/*void setCoilIRange(double min, double max)
{

}*/

void Experiment::setSampleI(double value)
{
    _sampleI_ = value;
}

void Experiment::setSampleThickness(double value)
{
    _sampleThickness_ = value;
}

void Experiment::stepSampleMeas_cd(Experiment *this_)
{
    double val(this_->readSingle());
    this_->dataUcd = val;
    this_->csvFile.setAt(Experiment::csvColSampleUcdF, val);

    stepOpenAllRoutes(this_);
}

void Experiment::stepSampleMeas_cdRev(Experiment *this_)
{
    double val(this_->readSingle());
    this_->dataUdc = val;
    this_->csvFile.setAt(Experiment::csvColSampleUcdB, val);

    stepOpenAllRoutes(this_);
}

void Experiment::stepSampleMeas_da(Experiment *this_)
{
    double val(this_->readSingle());
    this_->dataUda = val;
    this_->csvFile.setAt(Experiment::csvColSampleUdaF, val);

    stepOpenAllRoutes(this_);
}

void Experiment::stepSampleMeas_daRev(Experiment *this_)
{
    double val(this_->readSingle());
    this_->dataUad = val;
    this_->csvFile.setAt(Experiment::csvColSampleUdaB, val);

    stepOpenAllRoutes(this_);
}

void Experiment::stepSampleMeas_ac(Experiment *this_)
{
    double val(this_->readSingle());
    this_->dataUac = val;
    this_->csvFile.setAt(Experiment::csvColSampleUacF, val);

    stepOpenAllRoutes(this_);
}

void Experiment::stepSampleMeas_acRev(Experiment *this_)
{
    double val(this_->readSingle());
    this_->dataUca = val;
    this_->csvFile.setAt(Experiment::csvColSampleUacB, val);

    stepOpenAllRoutes(this_);
}

void Experiment::stepSampleMeas_bd(Experiment *this_)
{
    double val(this_->readSingle());
    this_->dataUbd = val;
    this_->csvFile.setAt(Experiment::csvColSampleUbdF, val);

    stepOpenAllRoutes(this_);
}

void Experiment::stepSampleMeas_bdRev(Experiment *this_)
{
    double val(this_->readSingle());
    this_->dataUdb = val;
    this_->csvFile.setAt(Experiment::csvColSampleUbdB, val);

    stepOpenAllRoutes(this_);
}

void Experiment::stepSampleMeasPrepare_cd(Experiment *this_)
{
    HP34970Hack::Channels_t scan;
    scan.append(Experiment::_34901A_sample_cd);
    this_->hp34970Hack.setScan(scan);
}

void Experiment::stepSampleMeasPrepare_da(Experiment *this_)
{
    HP34970Hack::Channels_t scan;
    scan.append(Experiment::_34901A_sample_da);
    this_->hp34970Hack.setScan(scan);
}

void Experiment::stepSampleMeasPrepare_ac(Experiment *this_)
{
    HP34970Hack::Channels_t scan;
    scan.append(Experiment::_34901A_sample_ac);
    this_->hp34970Hack.setScan(scan);
}

void Experiment::stepSampleMeasPrepare_bd(Experiment *this_)
{
    HP34970Hack::Channels_t scan;
    scan.append(Experiment::_34901A_sample_bd);
    this_->hp34970Hack.setScan(scan);
}

void Experiment::stepSamplePower_mp(Experiment *this_)
{
    this_->ps622Hack.setOutput(false);
    this_->ps622Hack.setCurrent(-(this_->_sampleI_));
}

void Experiment::stepSamplePower_pm(Experiment *this_)
{
    this_->ps622Hack.setOutput(false);

    this_->ps622Hack.setCurrent(this_->_sampleI_);
    this_->csvFile.setAt(Experiment::csvColSampleI, this_->_sampleI_);
}

void Experiment::stepSamplePower_ba(Experiment *this_)
{
    HP34970Hack::Channels_t channels;
    channels.append(_34903A_sample_a_pwr_m);
    channels.append(_34903A_sample_b_pwr_p);
    this_->hp34970Hack.setRoute(channels, _34903A);
    this_->ps622Hack.setOutput(true);
}

void Experiment::stepSamplePower_bc(Experiment *this_)
{
    HP34970Hack::Channels_t channels;
    channels.append(_34903A_sample_b_pwr_p);
    channels.append(_34903A_sample_c_pwr_sw1);
    channels.append(_34903A_pwr_sw1_pwr_m);
    this_->hp34970Hack.setRoute(channels, _34903A);
    this_->ps622Hack.setOutput(true);
}

void Experiment::stepSamplePower_bd(Experiment *this_)
{
    HP34970Hack::Channels_t channels;
    channels.append(_34903A_sample_b_pwr_p);
    channels.append(_34903A_sample_d_pwr_m);
    this_->hp34970Hack.setRoute(channels, _34903A);
    this_->ps622Hack.setOutput(true);
}

void Experiment::stepSamplePower_ca(Experiment *this_)
{
    HP34970Hack::Channels_t channels;
    channels.append(_34903A_sample_a_pwr_m);
    channels.append(_34903A_sample_c_pwr_sw1);
    channels.append(_34903A_pwr_sw1_pwr_p);
    this_->hp34970Hack.setRoute(channels, _34903A);
    this_->ps622Hack.setOutput(true);
}

void Experiment::stepAbort(Experiment *this_)
{
    this_->stepCurrent = this_->stepsRunning.end();
}

void Experiment::stepFinish(Experiment *this_)
{
    this_->_dataResistivity_ = ((this_->dataUcd - this_->dataUdc) +
                                (this_->dataUda - this_->dataUad)) / 4 / this_->_sampleI_;
    // TODO: kontrola rozptylu hodnot napětí
    this_->_dataResSpec_ = M_PI * this_->_sampleThickness_ / M_LN2 * this_->_dataResistivity_;

    this_->_dataRHall_ = ((this_->dataUca - this_->dataUac) +
                          (this_->dataUbd - this_->dataUdb)) / 4 - this_->_dataHallU0_;

    this_->_dataRHall_ = this_->_sampleThickness_ * (this_->_dataRHall_) / this_->_dataB_;

    this_->csvFile.setAt(Experiment::csvColSampleResistivity, this_->_dataResistivity_);
    this_->csvFile.setAt(Experiment::csvColSampleResSpec, this_->_dataResSpec_);
    this_->csvFile.setAt(Experiment::csvColSampleRHall, this_->_dataRHall_);
    //this_->csvFile.setAt(Experiment::csvColSampleDrift, this_->_dataDrift_);
    emit this_->measured(this_->csvFile.at(Experiment::csvColTime),
                         this_->_dataB_, this_->_dataRHall_, this_->_dataResistivity_);
    this_->csvFile.write();
}

void Experiment::stepGetTime(Experiment *this_)
{
    this_->csvFile.setAt(csvColTime, QDateTime::currentDateTimeUtc());
}

void Experiment::stepMeasHallProbe(Experiment *this_)
{
    double val(this_->readSingle());

    this_->csvFile.setAt(Experiment::csvColHallProbeU, val);
    this_->_dataB_ = this_->computeB(val);
    this_->csvFile.setAt(Experiment::csvColHallProbeB, this_->_dataB_);
    if (this_->_coilWantI_ == 0)
        this_->_dataHallU0_ = this_->_dataB_;

    stepOpenAllRoutes(this_);
}

void Experiment::stepMeasHallProbePrepare(Experiment *this_)
{
    /* set current to 1mA, open probe current source */
    this_->ps622Hack.setCurrent(hallProbeI);
    this_->csvFile.setAt(Experiment::csvColHallProbeI, hallProbeI);

    HP34970Hack::Channels_t closeChannels;
    closeChannels.append(_34903A_hall_probe_1_pwr_m);
    closeChannels.append(_34903A_hall_probe_2_pwr_p);
    this_->hp34970Hack.setRoute(closeChannels, _34903A);

    HP34970Hack::Channels_t scan;
    scan.append(Experiment::_34901A_hall_probe);
    this_->hp34970Hack.setScan(scan);

    this_->ps622Hack.setOutput(true);
}

void Experiment::stepOpenAllRoutes(Experiment *this_)
{
    HP34970Hack::Channels_t closeChannels;

    this_->hp34970Hack.setRoute(closeChannels, _34903A);
    this_->ps622Hack.setOutput(false);
}
