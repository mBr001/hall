#ifndef QCSVFILE_H
#define QCSVFILE_H

#include <QDateTime>
#include <QFile>
#include <QLocale>
#include <QVector>

class QCSVFile : public QVector<QString>
{
protected:
    /** Column separator. */
    QChar _cellSeparator_;
    /** Decimal separator used in file for floating point numbers. */
    QChar _decimalPoint_;
    /** File to write CSV data. */
    QFile file;
    bool firstRow;
    /** C locale, used to convert numbers to strig etc. */
    QLocale localeC;

public:
    QCSVFile();
    QCSVFile(int columns);
    QChar cellSeparator() { return _cellSeparator_; }
    void close();
    QChar decimalSeparator() {return _decimalPoint_; }
    QString errorString() const;
    bool open();
    const QString& setAt(const int index, const double &value);
    const QString& setAt(const int index, const int value) {
        return setAt(index, qint64(value)); }
    const QString& setAt(const int index, const unsigned int value) {
        return setAt(index, qint64(value)); }
    const QString& setAt(const int index, const long int value) {
        return setAt(index, qint64(value)); }
    const QString& setAt(const int index, const qint64 value);
    const QString& setAt(const int index, const QDateTime &value);
    const QString& setAt(const int index, const QString &value);
    void setCellSeparator(const QChar separator) { _cellSeparator_ = separator; }
    void setDecimalSeparator(const QChar separator) { _decimalPoint_ = separator; }
    void setFileName(QString name);
    bool write();
};

#endif // QCSVFILE_H
