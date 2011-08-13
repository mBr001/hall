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

const Experiment::Step_t Experiment::stepsMeasure[] = {
    // 2) after stepRestart function is done ++ and first step is therefore skipped
    {   NULL, 0 },
    {   stepSetNewTarget, 0 },
    {   stepSweeping, 500 },

    {   stepGetTime, 0 },
    {   stepMeasHallProbe, 0 },

    {   stepSamplePower_pm, 10 },
    {   stepSamplePower_bd, 10 },
    {   stepSampleMeasPrepare_ac, 100 },
    {   stepSampleMeas_ac, 10 },
    {   stepSamplePower_ca, 10 },
    {   stepSampleMeasPrepare_bd, 100 },
    {   stepSampleMeas_bd, 10 },

    {   stepSamplePower_ba, 10 },
    {   stepSampleMeasPrepare_cd, 100 },
    {   stepSampleMeas_cd, 10 },
    {   stepSamplePower_bc, 10 },
    {   stepSampleMeasPrepare_da, 100 },
    {   stepSampleMeas_da, 10 },

    {   stepSamplePower_mp, 10 },
    {   stepSamplePower_bd, 10 },
    {   stepSampleMeasPrepare_ac, 100 },
    {   stepSampleMeas_acRev, 10 },
    {   stepSamplePower_ca, 10 },
    {   stepSampleMeasPrepare_bd, 100 },
    {   stepSampleMeas_bdRev, 10 },

    {   stepSamplePower_ba, 10 },
    {   stepSampleMeasPrepare_cd, 100 },
    {   stepSampleMeas_cdRev, 10 },
    {   stepSamplePower_bc, 10 },
    {   stepSampleMeasPrepare_da, 100 },
    {   stepSampleMeas_daRev, 10 },

    {   stepFinish, 0 },
    {   stepAbortIfTargetReached, 0 },
    {   stepMeasHallProbePrepare, 10 },
    {   stepRestart, 0 },
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
    _coilIStep_(NAN),
    coilTimer(this),
    _coilWantI_(NAN),
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
    hp34970Dev.setRoute(ScpiDev::Channels_t());
    hp34970Dev.close();
    ps6220Dev.setOutput(false);
    // FIXME: hack, je to kvůli tomu aby si pamatoval posladní nastavené U pro vzorek
    ps6220Dev.setCurrent(_sampleI_);
    ps6220Dev.close();
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
    double B(B1 + sqrt(B2 + B3 * fabs(U / hallProbeI)));
    // alternativní vzorec a čísla
    // U /= I; B = U(A+sqrt(U)*(B+C*sqrt(U)))-D;
    // A=5.97622E-4 B=1.591394E-6 C=-9.24701E-11 D=-0.015

    return (U / hallProbeI) > 0. ? B : -B;
}

bool Experiment::isMeasuring()
{
    return _measuring_;
}

void Experiment::measure(bool single)
{
    _measuring_ = true;

    _measuringRange_.clear();
    if (!single) {
        _measuringRange_.append(0);

        double I(_coilIRangeTop_);
        for (; I >= 0 && I >= _coilIRangeBottom_; I -= _coilIStep_) {
            _measuringRange_.append(I);
        }
        if (I < _coilIRangeBottom_ || I < 0.)
            _measuringRange_.append(std::max(0., _coilIRangeBottom_));
        for (I = _coilIRangeBottom_; I <= 0 && I <= _coilIRangeTop_; I += _coilIStep_) {
            _measuringRange_.append(I);
        }
        if (I > _coilIRangeTop_ || I > 0.)
            _measuringRange_.append(std::min(0., _coilIRangeTop_));
    }
    stepsRunning = Steps_t(
                stepsMeasure,
                stepsMeasure + ARRAY_SIZE(stepsMeasure));
    stepCurrent = stepsRunning.begin();

    measTimer.start(0);
}

void Experiment::measurementAbort()
{
    if (_measuring_) {
        _measuring_ = false;
        measTimer.stop();
        emit measurementCompleted();
    }

    // Set up measurement to measure B at coil by default
    // used for periodical B measurement when idle
    if (ps6220Dev.output())
        ps6220Dev.setOutput(false);
    stepMeasHallProbePrepare(this);
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

        if (!_measuring_)
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
        if (stepCurrent->func != NULL) {
            stepCurrent->func(this);
        }
        int delay(stepCurrent->delay);
        ++stepCurrent;
        if (stepCurrent < stepsRunning.end()) {
            measTimer.start(delay);
            return;
        }
    }
    measurementAbort();
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

        csvFile[csvColHallProbeB] = "Hall probe\nB [T]";
        csvFile[csvColSampleResistivity] = "sample\nR [ohm]";
        csvFile[csvColSampleResSpec] = "sample\nRspec [ohm*m]";
        csvFile[csvColSampleRHall] = "sample\nRhall [m^3*C^-1]";
        csvFile[csvColSampleDrift] = "sample\ndrift [m^2*V^-1*s^-1]";

        csvFile[csvColTime] = "Time\n(UTC)";
        csvFile[csvColHallProbeU] = "Hall probe\nUhp [V]";
        csvFile[csvColSampleUacF] = "sample\nUac/+- [V]";
        csvFile[csvColSampleUacB] = "sample\nUac/-+ [V]";
        csvFile[csvColSampleUbdF] = "sample\nUbd/+- [V]";
        csvFile[csvColSampleUbdB] = "sample\nUbd/-+ [V]";
        csvFile[csvColSampleUcdF] = "sample\nUcd/+- [V]";
        csvFile[csvColSampleUcdB] = "sample\nUcd/-+ [V]";
        csvFile[csvColSampleUdaF] = "sample\nUda/+- [V]";
        csvFile[csvColSampleUdaB] = "sample\nUda/-+ [V]";

        csvFile[csvColBFormula] = "B formula\n" /* TODO */;
        csvFile[csvColHallProbeI] = "Hall proble\nIhp [A]";
        csvFile[csvColSampleI] = "sample\nI [A]";
        csvFile[csvColSampleThickness] = "Sample thickness\nh [um]";

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
        if (!ps6220Dev.open(port.toLocal8Bit().constData(), QSerial::Baude19200))
        {
            err = errno;
            throw new Error("Failed to open sample power supply (Keithaly 6220)",
                            QString::fromLocal8Bit(strerror(err)));
        }
        _sampleI_ = ps6220Dev.current();

        // Open and setup HP34970 device
        if (!hp34970Dev.open(config.hp34970Port())) {
            err = errno;
            throw new Error("Failed to open HP34970 device",
                            QString::fromLocal8Bit(strerror(err)));
        }
        ScpiDev::Channels_t channels;

        channels << 101 << 102 << 103 << 104 << 114;
        hp34970Dev.setSense(ScpiDev::SenseVolt, channels);
        // set up measurement of B
        measurementAbort();

        coilTimer.start();
    }
    catch (Error e)
    {
        csvFile.close();
        sdp_close(&sdp);
        pwrPolSwitch.close();
        ps6220Dev.close();

        throw;
    }
}

double Experiment::readSingle()
{
    QStringList data(hp34970Dev.read());
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
    return ps6220Dev.current();
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

void Experiment::setCoilIRange(double val1, double val2)
{
    if (val1 < val2) {
        _coilIRangeBottom_ = val1;
        _coilIRangeTop_ = val2;
    } else {
        _coilIRangeBottom_ = val2;
        _coilIRangeTop_ = val1;
    }
}

void Experiment::setCoilIStep(double val)
{
    _coilIStep_ = val;
}

void Experiment::setSampleI(double value)
{
    _sampleI_ = value;
}

void Experiment::setSampleThickness(double value)
{
    _sampleThickness_ = value;
}

void Experiment::stepRestart(Experiment *this_)
{
    this_->_sweeping_ = true;
    this_->stepCurrent = this_->stepsRunning.begin();
}

void Experiment::stepSampleMeas_cd(Experiment *this_)
{
    double val(this_->readSingle());
    this_->dataUcd = val;
    this_->csvFile.setAt(Experiment::csvColSampleUcdF, val);

    this_->ps6220Dev.setOutput(false);
}

void Experiment::stepSampleMeas_cdRev(Experiment *this_)
{
    double val(this_->readSingle());
    this_->dataUdc = val;
    this_->csvFile.setAt(Experiment::csvColSampleUcdB, val);

    this_->ps6220Dev.setOutput(false);
}

void Experiment::stepSampleMeas_da(Experiment *this_)
{
    double val(this_->readSingle());
    this_->dataUda = val;
    this_->csvFile.setAt(Experiment::csvColSampleUdaF, val);

    this_->ps6220Dev.setOutput(false);
}

void Experiment::stepSampleMeas_daRev(Experiment *this_)
{
    double val(this_->readSingle());
    this_->dataUad = val;
    this_->csvFile.setAt(Experiment::csvColSampleUdaB, val);

    this_->ps6220Dev.setOutput(false);
}

void Experiment::stepSampleMeas_ac(Experiment *this_)
{
    double val(this_->readSingle());
    this_->dataUac = val;
    this_->csvFile.setAt(Experiment::csvColSampleUacF, val);

    this_->ps6220Dev.setOutput(false);
}

void Experiment::stepSampleMeas_acRev(Experiment *this_)
{
    double val(this_->readSingle());
    this_->dataUca = val;
    this_->csvFile.setAt(Experiment::csvColSampleUacB, val);

    this_->ps6220Dev.setOutput(false);
}

void Experiment::stepSampleMeas_bd(Experiment *this_)
{
    double val(this_->readSingle());
    this_->dataUbd = val;
    this_->csvFile.setAt(Experiment::csvColSampleUbdF, val);

    this_->ps6220Dev.setOutput(false);
}

void Experiment::stepSampleMeas_bdRev(Experiment *this_)
{
    double val(this_->readSingle());
    this_->dataUdb = val;
    this_->csvFile.setAt(Experiment::csvColSampleUbdB, val);

    this_->ps6220Dev.setOutput(false);
}

void Experiment::stepSampleMeasPrepare_cd(Experiment *this_)
{
    this_->hp34970Dev.setScan(Experiment::_34901A_sample_cd);
}

void Experiment::stepSampleMeasPrepare_da(Experiment *this_)
{
    this_->hp34970Dev.setScan(Experiment::_34901A_sample_da);
}

void Experiment::stepSampleMeasPrepare_ac(Experiment *this_)
{
    this_->hp34970Dev.setScan(Experiment::_34901A_sample_ac);
}

void Experiment::stepSampleMeasPrepare_bd(Experiment *this_)
{
    this_->hp34970Dev.setScan(Experiment::_34901A_sample_bd);
}

void Experiment::stepSamplePower_mp(Experiment *this_)
{
    this_->ps6220Dev.setCurrent(-(this_->_sampleI_));
}

void Experiment::stepSamplePower_pm(Experiment *this_)
{
    this_->ps6220Dev.setCurrent(this_->_sampleI_);
    this_->csvFile.setAt(Experiment::csvColSampleI, this_->_sampleI_);
}

void Experiment::stepSamplePower_ba(Experiment *this_)
{
    ScpiDev::Channels_t channels;
    channels.append(_34903A_sample_a_pwr_m);
    channels.append(_34903A_sample_b_pwr_p);
    this_->hp34970Dev.setRoute(channels);
    this_->ps6220Dev.setOutput(true);
}

void Experiment::stepSamplePower_bc(Experiment *this_)
{
    ScpiDev::Channels_t channels;
    channels.append(_34903A_sample_b_pwr_p);
    channels.append(_34903A_sample_c_pwr_sw1);
    channels.append(_34903A_pwr_sw1_pwr_m);
    this_->hp34970Dev.setRoute(channels);
    this_->ps6220Dev.setOutput(true);
}

void Experiment::stepSamplePower_bd(Experiment *this_)
{
    ScpiDev::Channels_t channels;
    channels.append(_34903A_sample_b_pwr_p);
    channels.append(_34903A_sample_d_pwr_m);
    this_->hp34970Dev.setRoute(channels);
    this_->ps6220Dev.setOutput(true);
}

void Experiment::stepSamplePower_ca(Experiment *this_)
{
    ScpiDev::Channels_t channels;
    channels.append(_34903A_sample_a_pwr_m);
    channels.append(_34903A_sample_c_pwr_sw1);
    channels.append(_34903A_pwr_sw1_pwr_p);
    this_->hp34970Dev.setRoute(channels);
    this_->ps6220Dev.setOutput(true);
}

void Experiment::stepAbortIfTargetReached(Experiment *this_)
{
    if (!this_->_measuringRange_.size())
        this_->stepCurrent = this_->stepsRunning.end();
}

void Experiment::stepFinish(Experiment *this_)
{
    this_->_dataResistivity_ = ((this_->dataUcd - this_->dataUdc) +
                                (this_->dataUda - this_->dataUad)) / 4 / this_->_sampleI_;
    // TODO: kontrola rozptylu hodnot napětí
    this_->_dataResSpec_ = M_PI * this_->_sampleThickness_ / M_LN2 * this_->_dataResistivity_;

    double hallU(((this_->dataUca - this_->dataUac) +
                  (this_->dataUbd - this_->dataUdb)) / 4 - this_->_dataHallU0_);

    this_->_dataRHall_ = this_->_sampleThickness_ * hallU / this_->_dataB_;

    this_->csvFile.setAt(Experiment::csvColSampleResistivity, this_->_dataResistivity_);
    this_->csvFile.setAt(Experiment::csvColSampleResSpec, this_->_dataResSpec_);
    this_->csvFile.setAt(Experiment::csvColSampleRHall, this_->_dataRHall_);
    //this_->csvFile.setAt(Experiment::csvColSampleDrift, this_->_dataDrift_);
    this_->csvFile.setAt(Experiment::csvColSampleDrift, "TODO");
    emit this_->measured(this_->csvFile.at(Experiment::csvColTime),
                         this_->_dataB_, this_->_dataResistivity_, hallU);

    QString eq("B=%1 + sqrt(%2 + %3 * Uhp / Ihp)");
    this_->csvFile.setAt(Experiment::csvColBFormula, eq.arg(this_->B1).arg(this_->B2).arg(this_->B3));
    this_->csvFile.setAt(Experiment::csvColSampleThickness, this_->_sampleThickness_);
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
    this_->csvFile.setAt(Experiment::csvColHallProbeI, hallProbeI);
    if (this_->_coilWantI_ == 0)
        this_->_dataHallU0_ = this_->_dataB_;

    this_->ps6220Dev.setOutput(false);
}

void Experiment::stepMeasHallProbePrepare(Experiment *this_)
{
    /* set current to 1mA, open probe current source */
    this_->ps6220Dev.setCurrent(hallProbeI);

    ScpiDev::Channels_t closeChannels;
    closeChannels.append(_34903A_hall_probe_1_pwr_m);
    closeChannels.append(_34903A_hall_probe_2_pwr_p);
    this_->hp34970Dev.setRoute(closeChannels);

    this_->hp34970Dev.setScan(Experiment::_34901A_hall_probe);

    this_->ps6220Dev.setOutput(true);
}

void Experiment::stepSetNewTarget(Experiment *this_)
{
    if (this_->_measuringRange_.size()) {
        this_->_coilWantI_ = this_->_measuringRange_.front();
        this_->_measuringRange_.pop_front();
        this_->_sweeping_ = true;
    }
}

void Experiment::stepSweeping(Experiment *this_)
{
    if (this_->_sweeping_) {
        --(this_->stepCurrent);
    }
}
