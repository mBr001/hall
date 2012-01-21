#include <errno.h>
#include <math.h>

#include "experiment.h"
#include "vanderpauwsolver.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#endif

const int Experiment::_34901A = 100;
const int Experiment::_34901A_sample_cd = _34901A + 1;
const int Experiment::_34901A_sample_da = _34901A + 2;
const int Experiment::_34901A_sample_bd = _34901A + 3;
const int Experiment::_34901A_sample_ac = _34901A + 4;
const int Experiment::_34901A_hall_probe = _34901A + 5;

const int Experiment::_34903A = 200;
const int Experiment::_34903A_min = _34903A + 1;
const int Experiment::_34903A_max = _34903A + 20;
const int Experiment::_34903A_sample_a_pwr_m = _34903A + 1;
const int Experiment::_34903A_sample_b_pwr_p = _34903A + 2;
const int Experiment::_34903A_sample_c_pwr_sw1 = _34903A + 3;
const int Experiment::_34903A_sample_d_pwr_m = _34903A + 4;
const int Experiment::_34903A_pwr_sw1_pwr_m = _34903A + 5;
const int Experiment::_34903A_pwr_sw1_pwr_p = _34903A + 6;
const int Experiment::_34903A_hall_probe_1_pwr_m = _34903A + 9;
const int Experiment::_34903A_hall_probe_2_pwr_p = _34903A + 10;

const double Experiment::carriercUnits = 1e6; // cm^-3
const double Experiment::hallProbeI = 0.001; // 1 mA
const double Experiment::hallProbeIUnits = 1e-3; // mA
const double Experiment::q = 1.602176565e-19; // e- [C]
const double Experiment::sampleThicknessUnits = 1e-6; // um

const QString Experiment::eqationBScript(
                 "I=%1;\n"
                 "U=%2;\n"

                 "E=Math.E;\n"
                 "LOG2E=Math.LOG2E;\n"
                 "LOG10E=Math.LOG10E;\n"
                 "LN2=Math.LN2;\n"
                 "LN10=Math.LN10;\n"
                 "SQRT2=Math.SQRT2;\n"
                 "SQRT1_2=Math.SQRT1_2;\n"

                 "abs=Math.abs;\n"
                 "acos=Math.acos;\n"
                 "asin=Math.asin;\n"
                 "atan=Math.atan;\n"
                 "atan2=Math.atan2;\n"
                 "ceil=Math.ceil;\n"
                 "cos=Math.cos;\n"
                 "exp=Math.exp;\n"
                 "floor=Math.floor;\n"
                 "log=Math.log;\n"
                 "max=Math.max;\n"
                 "min=Math.min;\n"
                 "pow=Math.pow;\n"
                 "random=Math.random;\n"
                 "round=Math.round;\n"
                 "sin=Math.sin;\n"
                 "sqrt=Math.sqrt;\n"
                 "tan=Math.tan;\n"

                 "%3;\n"
                 "B");

const Experiment::Step_t Experiment::stepsMeasure[] = {
    // 1) after stepRestart function is done ++ and first step is therefore skipped
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

Experiment::Experiment(Config *config, QObject *parent) :
    QObject(parent),
    _coilIStep_(NAN),
    coilTimer(this),
    _coilWantI_(NAN),
    measTimer(this),
    _measuring_(false),
    _sweeping_(false),
    _sampleI_(0)
{
    this->config = config;

    coilTimer.setObjectName("coilTimer");
    coilTimer.setInterval(currentDwell);
    coilTimer.setSingleShot(false);

    measTimer.setObjectName("measTimer");
    measTimer.setSingleShot(true);

    QMetaObject::connectSlotsByName(this);

    dataB.reserve(1024);
    dataHallU.reserve(1024);
    dataResistivity.reserve(1024);
}

// TODO
bool Experiment::checkSettings()
{
    return false;
}

void Experiment::close()
{
    coilTimer.stop();
    measTimer.stop();
    csvFile.close();
    pwrPolSwitch.close();
    sdp_close(&sdp);
    ps6220Dev.setOutput(false);
    ps6220Dev.close();
    hp34970Dev.setRoute(QSCPIDev::Channels_t(), _34903A_min, _34903A_max);
    hp34970Dev.close();
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

    QString eq(eqationBScript.arg(hallProbeI).arg(U).arg(_equationB_));
    scriptEngine.clearExceptions();
    QScriptValue result(scriptEngine.evaluate(eq));
    if (scriptEngine.hasUncaughtException())
        return NAN;

    qsreal B(result.toNumber());
    //double B(B1 + sqrt(B2 + B3 * fabs(U / hallProbeI)));
    // alternativní vzorec a čísla
    // U /= I; B = U*(A+sqrt(U)*(B+C*sqrt(U)))-D;
    // A=5.97622E-4 B=1.591394E-6 C=-9.24701E-11 D=-0.015

    return B;
}

int Experiment::ETA()
{
    if (isMeasuring()) {
        return -1;
    }
    // Coil current range pass trought
    double t_coil_swepping = (_coilIRangeTop_ > 0 ? (2 * _coilIRangeTop_) : 0) +
            (_coilIRangeBottom_ < 0 ? (2 * -_coilIRangeBottom_) : 0);
    // Coil swepping time from current range
    t_coil_swepping /= currentSlope;

    // crossing 0.0V takes 3 steps???
    if (_coilIRangeTop_ > 0)
        t_coil_swepping += 6 * currentDwell / 1000;
    if (_coilIRangeBottom_ < 0)
        t_coil_swepping += 6 * currentDwell / 1000;

    int t_sample = 0;
    // TODO
    return t_coil_swepping + t_sample;
}

QString Experiment::filePath()
{
    QString dateStr(QDateTime::currentDateTimeUtc()
                    .toString(Qt::ISODate)
                    .replace(":", "."));
    QString nameStr(config->sampleName()
                    .replace(QRegExp("[*-/\\@#$%^&()=\"\'!§?<>`;]"), "_"));
    QString fileName(dateStr + "_" + nameStr + ".csv");

    return QDir(config->dataDirPath()).filePath(fileName);
}

const QVector<double> &Experiment::getDataB()
{
    return dataB;
}

const QVector<double> &Experiment::getDataHallU()
{
    return dataHallU;
}

const QVector<double> &Experiment::getDataResistivity()
{
    return dataResistivity;
}

bool Experiment::isMeasuring()
{
    return _measuring_;
}

std::pair<double, double> Experiment::linRegress(const QVector<double> &x,
                                                 const QVector<double> &y)
{
    if (x.size() != y.size() || x.size() < 2)
        return std::make_pair<double, double>(NAN, NAN);

    double n(x.size());
    double sum_x(0.), sum_y(0.), sum_x2(0.), sum_x_y(0.);

    foreach(double i, x) { sum_x += i; sum_x2 += i*i; }
    foreach(double i, y) { sum_y += i; }
    for(QVector<double>::const_iterator xi(x.begin()), yi(y.begin());
            xi != x.end();
            ++xi, ++yi) {
        sum_x_y += *xi * *yi;
    }

    double a = (n*sum_x_y - sum_x*sum_y)/(n*sum_x2 - sum_x*sum_x);
    double b = (sum_x2*sum_y - sum_x*sum_x_y)/(n*sum_x2 - sum_x*sum_x);

    return std::make_pair<double, double>(a, b);
}

void Experiment::measure(bool single)
{
    _measuring_ = true;
    _sampleI_ = config->sampleI();

    _measuringRange_.clear();
    if (!single) {
        const double eps = 0.0001;
        _measuringRange_.append(_coilIRangeBottom_);

        for (int n(1); n; --n) {
            double I;

            for (I = _coilIRangeBottom_ + _coilIStep_;
                    I - _coilIRangeTop_<= eps;
                    I += _coilIStep_)
                _measuringRange_.append(I);

            for (I = _coilIRangeTop_ - _coilIStep_;
                    I - _coilIRangeBottom_ >= -eps;
                    I -= _coilIStep_)
                _measuringRange_.append(I);
        }
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
    bool open;
    ps6220Dev.output(&open);
    if (open) {
        ps6220Dev.setOutput(false);
    }
    stepMeasHallProbePrepare(this);
}

void Experiment::on_coilTimer_timeout()
{
    sdp_lcd_info_t lcd_info;

    int err(sdp_get_lcd_info(&sdp, &lcd_info));
    if (err < 0) {
        emit fatalError("on_coilTimer_timeout - sdp_get_lcd_info",
                        sdp_strerror(err));
        return;
    }

    emit coilIMeasured(lcd_info.read_A);
    emit coilUMeasured(lcd_info.read_V);

    // if no measurement in progress we measure B
    if (!_measuring_) {
        double U(readSingle());
        if (isnan(U))
            return;
        emit coilBMeasured(computeB(U));
    }

    // update coil current value
    if (!_sweeping_)
        return;

    /** Curent through coil. */
    double procI;
    /** Coil power state, on/off. */
    bool procCoilPower, wantCoilPower;
    /** Coil power switch state direct/cross. */
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
            int err(sdp_set_output(&sdp, 0));
            if (err < 0) {
                emit fatalError("on_coilTimer_timeout - sdp_set_output",
                                sdp_strerror(err));
            }
            return;
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
        if (stepCurrent != stepsRunning.end()) {
            int delay(stepCurrent->delay);
            ++stepCurrent;
            if (stepCurrent != stepsRunning.end()) {
                measTimer.start(delay);
                return;
            }
        }
    }
    measurementAbort();
}

bool Experiment::open()
{
    int err;
    QString port;

    _equationB_ = config->hallProbeEquationB(config->selectedSampleHolderName());
    // just hack to check validity of equation, suppose that for at least one of
    // { -1, 0, 1 } must provide valid result
    if (!isfinite(computeB(-1)) && !isfinite(computeB(0)) && !isfinite(computeB(1))) {
        QString msg("Equation for magnetic field intensity is wrong, please fix it.\n\nerror: %1");
        emit fatalError("Failed to evaluate B equation",
                        msg.arg(scriptEngine.uncaughtException().toString()));
        return false;
    }

    _sampleThickness_ = config->sampleThickness();

    port = config->msdpPort();
    err = sdp_open(&sdp, port.toLocal8Bit().constData(), SDP_DEV_ADDR_MIN);
    if (err < 0) {
        emit fatalError("Manson SDP power supply open failed",
                        QString::fromLocal8Bit(sdp_strerror(err)));
        goto err;
    }

    /* Set value limit in current input spin box. */
    sdp_va_t limits;
    err = sdp_get_va_maximums(&sdp, &limits);
    if (err < 0) {
        emit fatalError("Manson SDP power supply operation failed",
                        QString::fromLocal8Bit(sdp_strerror(err)));
        goto err_sdp;
    }
    _coilMaxI_ = limits.curr;

    /* Set actual current value as wanted value, avoiding unwanted hickups. */
    sdp_va_data_t va_data;
    err = sdp_get_va_data(&sdp, &va_data);
    if (err < 0) {
        emit fatalError("Manson SDP power supply operation failed",
                        QString::fromLocal8Bit(sdp_strerror(err)));
        goto err_sdp;
    }
    _coilWantI_ = va_data.curr;

    err = sdp_set_curr(&sdp, va_data.curr);
    if (err < 0) {
        emit fatalError("Manson SDP power supply operation failed",
                        QString::fromLocal8Bit(sdp_strerror(err)));
        goto err_sdp;
    }

    /* Set voltage to maximum, we operate only with current. */
    err = sdp_set_volt_limit(&sdp, limits.volt);
    if (err < 0) {
        emit fatalError("Manson SDP power supply operation failed",
                        QString::fromLocal8Bit(sdp_strerror(err)));
        goto err_sdp;
    }

    err = sdp_set_volt(&sdp, limits.volt);
    if (err < 0) {
        emit fatalError("Manson SDP power supply operation failed",
                        QString::fromLocal8Bit(sdp_strerror(err)));
        goto err_sdp;
    }

    sdp_lcd_info_t lcd_info;
    err = sdp_get_lcd_info(&sdp, &lcd_info);
    if (err < 0) {
        emit fatalError("Manson SDP power supply operation failed",
                        QString::fromLocal8Bit(sdp_strerror(err)));
        goto err_sdp;
    }
    if (!lcd_info.output)
        _coilWantI_ = 0;

    if (!csvFile.open(filePath())) {
        emit fatalError("Failed to open data file",
                        csvFile.errorString());
        goto err_sdp;
    }

    do {
        csvFile.resize(1);
        csvFile[0] = "Hall measurement experimental data";
        if (!csvFile.write())
            break;
        csvFile.error();

        csvFile.resize(2);
        csvFile[0] = "Date";
        csvFile[1] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        if (!csvFile.write())
            break;

        csvFile[0] = "Sample name";
        csvFile[1] = config->sampleName();
        if (!csvFile.write())
            break;

        csvFile[0] = "B formula";
        csvFile[1] = _equationB_;
        if (!csvFile.write())
            break;

        csvFile[0] = "Sample thickness [μm]";
        csvFile[1] = _sampleThickness_ / sampleThicknessUnits;
        if (!csvFile.write())
            break;

        csvFile[0] = "Hall probe I [mA]";
        csvFile[1] = hallProbeI / hallProbeIUnits;
        if (!csvFile.write())
            break;

        csvFile.resize(csvColEnd);

        csvFile[csvColHallProbeB] = "Hall probe\nB [T]";
        csvFile[csvColSampleResistivity] = "sample\nR [ohm]";
        csvFile[csvColSampleResSpec] = "sample\nRspec [ohm*m]";
        csvFile[csvColSampleRHall] = "sample\nRhall [m^3*C^-1]";
        csvFile[csvColSampleDrift] = "sample\ndrift [m^2*V^-1*s^-1]";
        csvFile[csvColSamplecCarrier] = "carrier conc.\nN [cm^-3]";

        csvFile[csvColResultsEnd] = "-";

        csvFile[csvColTime] = "Time\n(UTC)";
        csvFile[csvColTime].setDateTimeFormat("yyyy-MM-dd hh:mm:ss");
        csvFile[csvColHallProbeU] = "Hall probe\nUhp [V]";
        csvFile[csvColSampleUac] = "sample\nUac [V]";
        csvFile[csvColSampleUacRev] = "sample\nUac(rev) [V]";
        csvFile[csvColSampleUbd] = "sample\nUbd [V]";
        csvFile[csvColSampleUbdRev] = "sample\nUbd(rev) [V]";
        csvFile[csvColSampleUcd] = "sample\nUcd [V]";
        csvFile[csvColSampleUcdRev] = "sample\nUcd(rev) [V]";
        csvFile[csvColSampleUda] = "sample\nUda [V]";
        csvFile[csvColSampleUdaRev] = "sample\nUda(rev) [V]";

        csvFile[csvColSampleI] = "sample\nI [A]";
        csvFile[csvColCoilI] = "Coil\nI [A]";
        if (!csvFile.write())
            break;
    } while(false);

    if (csvFile.error() != QFile::NoError) {
        emit fatalError("Failed to write header into data file",
                        csvFile.errorString());
        goto err_csv_file;
    }

    // Open polarity switch device
    port = config->polSwitchPort();
    if (!pwrPolSwitch.open(port.toLocal8Bit().constData())) {
        err = errno;
        emit fatalError("Failed to open coil polarity switch port",
                        QString::fromLocal8Bit(strerror(err)));
        goto err_csv_file;
    }

    if (pwrPolSwitch.polarity() == PwrPolSwitch::cross) {
        _coilWantI_ = -_coilWantI_;
    }

    // Open sample power source
    if (!ps6220Dev.open(config->ps6220Port(), QSerial::Baude19200))
    {
        emit fatalError("Failed to open sample power supply (Keithaly 6220)",
                        ps6220Dev.errorString());
        goto err_pwr_pol_switch;
    }

    if (!ps6220Dev.current(&_sampleI_)) {
        emit fatalError("Failed to get current from Keithaly 6220",
                        ps6220Dev.errorString());
        goto err_ps6220dev;
    }

    // Open and setup HP34970 device
    if (!hp34970Dev.open(config->hp34970Port())) {
        emit fatalError("Failed to open HP34970 device", hp34970Dev.errorString());
        goto err_ps6220dev;
    }

    {
        QSCPIDev::Channels_t channels;
        channels << _34901A_sample_cd
                 << _34901A_sample_da
                 << _34901A_sample_bd
                 << _34901A_sample_ac
                 << _34901A_hall_probe;
        if (!hp34970Dev.setSense(QSCPIDev::SenseVolt, channels)) {
            emit fatalError("Failed set up HP34970 device", hp34970Dev.errorString());
            goto err_hp34970;
        }
        // set experiment to well defined idle state
        if (!reset())
            goto err_hp34970;

        coilTimer.start();

        return true;
    }

err_hp34970:
    hp34970Dev.close();

err_ps6220dev:
    ps6220Dev.close();

err_pwr_pol_switch:
    pwrPolSwitch.close();

err_csv_file:
    csvFile.close();

err_sdp:
    sdp_close(&sdp);

err:
    return false;
}

double Experiment::readSingle()
{
    QStringList values;
    if (!hp34970Dev.read(&values))
    {
        emit fatalError("HP34970 error", hp34970Dev.errorString());
        return NAN;
    }
    if (values.size() != 1) {
        emit fatalError("HP34970 error value error",
                        "unexpected number of values returned");
        return NAN;
    }

    bool ok;
    double val(QVariant(values[0]).toDouble(&ok));
    if (!ok) {
        emit fatalError("HP34970 error value error",
                        "failed to parse value as loating point number");
        return NAN;
    }
    return val;
}

bool Experiment::reset()
{
    measurementAbort();

    dataB.resize(0);
    if (!csvFile.write()) {
        emit fatalError("Failed to write header into data file",
                    csvFile.errorString());
        return false;
    }

    dataHallU.resize(0);
    dataResistivity.resize(0);

    csvFile.resize(0);
    csvFile.resize(csvColEnd);

    csvFile[csvColHallProbeB] = "Hall probe\nB [T]";
    csvFile[csvColSampleResistivity] = "sample\nR [ohm]";
    csvFile[csvColSampleResSpec] = "sample\nRspec [ohm*m]";
    csvFile[csvColSampleRHall] = "sample\nRhall [m^3*C^-1]";
    csvFile[csvColSampleDrift] = "sample\ndrift [m^2*V^-1*s^-1]";
    csvFile[csvColSamplecCarrier] = "carrier conc.\nN [cm^-3]";

    csvFile[csvColResultsEnd] = "-";

    csvFile[csvColTime] = "Time\n(UTC)";
    csvFile[csvColTime].setDateTimeFormat("yyyy-MM-dd hh:mm:ss");
    csvFile[csvColHallProbeU] = "Hall probe\nUhp [V]";
    csvFile[csvColSampleUac] = "sample\nUac [V]";
    csvFile[csvColSampleUacRev] = "sample\nUac(rev) [V]";
    csvFile[csvColSampleUbd] = "sample\nUbd [V]";
    csvFile[csvColSampleUbdRev] = "sample\nUbd(rev) [V]";
    csvFile[csvColSampleUcd] = "sample\nUcd [V]";
    csvFile[csvColSampleUcdRev] = "sample\nUcd(rev) [V]";
    csvFile[csvColSampleUda] = "sample\nUda [V]";
    csvFile[csvColSampleUdaRev] = "sample\nUda(rev) [V]";

    csvFile[csvColSampleI] = "sample\nI [A]";
    csvFile[csvColCoilI] = "Coil\nI [A]";
    if (!csvFile.write()) {
        emit fatalError("Failed to write header into data file",
                    csvFile.errorString());
        return false;
    }

    return true;
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

void Experiment::stepRestart(Experiment *this_)
{
    this_->_sweeping_ = true;
    this_->stepCurrent = this_->stepsRunning.begin();
}

void Experiment::stepSampleMeas_cd(Experiment *this_)
{
    double val(this_->readSingle());
    this_->dataUcd = val;
    this_->csvFile[csvColSampleUcd] = val;

    this_->ps6220Dev.setOutput(false);
}

void Experiment::stepSampleMeas_cdRev(Experiment *this_)
{
    double val(this_->readSingle());
    this_->dataUcdRev = val;
    this_->csvFile[csvColSampleUcdRev] = val;

    this_->ps6220Dev.setOutput(false);
}

void Experiment::stepSampleMeas_da(Experiment *this_)
{
    double val(this_->readSingle());
    this_->dataUda = val;
    this_->csvFile[csvColSampleUda] = val;

    this_->ps6220Dev.setOutput(false);
}

void Experiment::stepSampleMeas_daRev(Experiment *this_)
{
    double val(this_->readSingle());
    this_->dataUdaRev = val;
    this_->csvFile[csvColSampleUdaRev] = val;

    this_->ps6220Dev.setOutput(false);
}

void Experiment::stepSampleMeas_ac(Experiment *this_)
{
    double val(this_->readSingle());
    this_->dataUac = val;
    this_->csvFile[csvColSampleUac] = val;

    this_->ps6220Dev.setOutput(false);
}

void Experiment::stepSampleMeas_acRev(Experiment *this_)
{
    double val(this_->readSingle());
    this_->dataUacRev = val;
    this_->csvFile[csvColSampleUacRev] = val;

    this_->ps6220Dev.setOutput(false);
}

void Experiment::stepSampleMeas_bd(Experiment *this_)
{
    double val(this_->readSingle());
    this_->dataUbd = val;
    this_->csvFile[csvColSampleUbd] = val;

    this_->ps6220Dev.setOutput(false);
}

void Experiment::stepSampleMeas_bdRev(Experiment *this_)
{
    double val(this_->readSingle());
    this_->dataUbdRev = val;
    this_->csvFile[csvColSampleUbdRev] = val;

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
    this_->csvFile[csvColSampleI] = this_->_sampleI_;
}

void Experiment::stepSamplePower_ba(Experiment *this_)
{
    QSCPIDev::Channels_t channels;
    channels.append(_34903A_sample_a_pwr_m);
    channels.append(_34903A_sample_b_pwr_p);
    this_->hp34970Dev.setRoute(channels, _34903A_min, _34903A_max);
    this_->ps6220Dev.setOutput(true);
}

void Experiment::stepSamplePower_bc(Experiment *this_)
{
    QSCPIDev::Channels_t channels;
    channels.append(_34903A_sample_b_pwr_p);
    channels.append(_34903A_sample_c_pwr_sw1);
    channels.append(_34903A_pwr_sw1_pwr_m);
    this_->hp34970Dev.setRoute(channels, _34903A_min, _34903A_max);
    this_->ps6220Dev.setOutput(true);
}

void Experiment::stepSamplePower_bd(Experiment *this_)
{
    QSCPIDev::Channels_t channels;
    channels.append(_34903A_sample_b_pwr_p);
    channels.append(_34903A_sample_d_pwr_m);
    this_->hp34970Dev.setRoute(channels, _34903A_min, _34903A_max);
    this_->ps6220Dev.setOutput(true);
}

void Experiment::stepSamplePower_ca(Experiment *this_)
{
    QSCPIDev::Channels_t channels;
    channels.append(_34903A_sample_a_pwr_m);
    channels.append(_34903A_sample_c_pwr_sw1);
    channels.append(_34903A_pwr_sw1_pwr_p);
    this_->hp34970Dev.setRoute(channels, _34903A_min, _34903A_max);
    this_->ps6220Dev.setOutput(true);
}

void Experiment::stepAbortIfTargetReached(Experiment *this_)
{
    if (!this_->_measuringRange_.size())
        this_->stepCurrent = this_->stepsRunning.end();
}

void Experiment::stepFinish(Experiment *this_)
{
    // TODO: check B vs -B (B direction)

    double Uac(this_->dataUac - this_->dataUacRev);
    double Ubd(this_->dataUbd - this_->dataUbdRev);
    double Ucd(this_->dataUcd - this_->dataUcdRev);
    double Uda(this_->dataUdaRev - this_->dataUda);

    double Rdc(Ucd / this_->_sampleI_ / 2.);
    double Rad(Uda / this_->_sampleI_ / 2.);

    std::pair<double, double> resisitivity(VanDerPauwSolver::solve(Rdc, Rad));
    this_->_dataResistivity_ = resisitivity.first;
    this_->_dataResSpec_ = resisitivity.first * this_->_sampleThickness_;

    /* Uh = (Uac - Uca + (Ubd - Udb)) / 4    Uh - hall voltage,
      this equation is a bit twinkle because B x I x U orientation in space. */
    double hallU((Uac + Ubd) / 4);

    this_->dataB.append(this_->_dataB_);
    this_->dataHallU.append(hallU);
    this_->dataResistivity.append(this_->_dataResistivity_);

    /* Rhall = w * Uh / (B * I) */
    this_->_dataRHall_ = this_->_sampleThickness_ * hallU / this_->_dataB_ / this_->_sampleI_;

    // TODO: check this math
    /* um = Rh / (Rs * w) */
    this_->_dataDrift_ = fabs(this_->_dataRHall_ / (this_->_dataResistivity_ * this_->_sampleThickness_));

    std::pair<double, double> a_b(linRegress(this_->dataHallU, this_->dataB));

    double carrierConc = fabs(this_->_sampleI_ * a_b.first / (q * this_->_sampleThickness_));

    this_->csvFile[csvColSampleResistivity] = this_->_dataResistivity_;
    this_->csvFile[csvColSampleResSpec] = this_->_dataResSpec_;
    this_->csvFile[csvColSampleRHall] = this_->_dataRHall_;
    this_->csvFile[csvColSampleDrift] = this_->_dataDrift_;
    this_->csvFile[csvColSamplecCarrier] = carrierConc * carriercUnits;

    double errAsymetry, errShottky;

    errAsymetry = fabs(Ucd - Uda) / (Ucd + Uda);
    errShottky = std::max(fabs(this_->dataUac + this_->dataUacRev) / Uac,
                          std::max(fabs(this_->dataUbd + this_->dataUbdRev) / Ubd,
                                   std::max(fabs(this_->dataUcd + this_->dataUcdRev) / Ucd,
                                            fabs(this_->dataUda + this_->dataUdaRev) / Uda)));
    emit this_->measured(this_->_dataB_, hallU, this_->_dataResistivity_,
                         this_->_dataResSpec_, carrierConc, this_->_dataDrift_, errAsymetry, errShottky);

    this_->csvFile[csvColCoilI] = this_->_coilWantI_;
    this_->csvFile.write();
}

void Experiment::stepGetTime(Experiment *this_)
{
    this_->csvFile[csvColTime] = QDateTime::currentDateTimeUtc();
}

void Experiment::stepMeasHallProbe(Experiment *this_)
{
    double val(this_->readSingle());

    this_->csvFile[csvColHallProbeU] = val;
    this_->_dataB_ = this_->computeB(val);
    this_->csvFile[csvColHallProbeB] = this_->_dataB_;

    this_->ps6220Dev.setOutput(false);
}

void Experiment::stepMeasHallProbePrepare(Experiment *this_)
{
    /* Set current to 1mA, open hall probe current source. */
    this_->ps6220Dev.setCurrent(hallProbeI);

    QSCPIDev::Channels_t closeChannels;
    closeChannels.append(_34903A_hall_probe_1_pwr_m);
    closeChannels.append(_34903A_hall_probe_2_pwr_p);

    this_->hp34970Dev.setRoute(closeChannels, _34903A_min, _34903A_max);
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
        --this_->stepCurrent;
    }
}
