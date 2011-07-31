#include "qcsvfile.h"
#include <QStringList>

QCSVFile::QCSVFile() :
    QVector<QString>(),
    _cellSeparator_(','),
    _decimalPoint_('.'),
    file(),
    firstRow(true),
    localeC(QLocale::c())
{
}

QCSVFile::QCSVFile(int columns) :
    QVector<QString>(columns),
    _cellSeparator_(','),
    _decimalPoint_('.'),
    file(),
    firstRow(true),
    localeC(QLocale::c())
{
}

void QCSVFile::close()
{
    file.close();
}

QString QCSVFile::errorString() const
{
    return file.errorString();
}

bool QCSVFile::open()
{
    return file.open(QFile::WriteOnly | QFile::Truncate);
}

const QString& QCSVFile::setAt(const int index, const double &value)
{
    QString cell(localeC.toString(value));

    if (!_decimalPoint_.isNull() &&
            _decimalPoint_ != localeC.decimalPoint()) {
        cell = cell.replace(localeC.decimalPoint(), _decimalPoint_);
    }

    return (*this)[index] = cell;
}

const QString& QCSVFile::setAt(const int index, const qint64 value)
{
    return (*this)[index] = localeC.toString(value);
}

const QString& QCSVFile::setAt(const int index, const QDateTime &value)
{
    return (*this)[index] = value.toString("yyyy-MM-dd hh:mm:ss");
}

const QString& QCSVFile::setAt(const int index, const QString &value)
{
    return (*this)[index] = value;
}

void QCSVFile::setFileName(QString name)
{
    file.setFileName(name);
}

bool QCSVFile::write()
{
    QStringList row;
    qint64 wsize;

    row.reserve(size());
    for (QVector<QString>::iterator icells(begin());
         icells != end(); ++icells) {

        if (icells->contains(_cellSeparator_) || icells->contains("\"")) {
            *icells = QString("\"%1\"").arg(icells->replace("\"", "\"\""));
        }
        row.append(*icells);
        icells->clear();
    }

    QString rowS(row.join(_cellSeparator_));
    if (!firstRow) {
        rowS = QString("\r\n") + rowS;
    }
    firstRow = false;

    QByteArray array(rowS.toUtf8());
    wsize = file.write(array);
    if (wsize != array.size())
        return false;

    return true;
}
