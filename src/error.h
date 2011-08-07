#ifndef ERROR_H
#define ERROR_H

#include <QtCore>
#include <stdexcept>

/** Error class for error at application runtime. */
class Error : public std::exception
{
private:
    QString _description_;
    QString _longDescription_;

public:
    Error(const QString &description, const QString &longDescription = QString());
    ~Error() throw() {}
    const QString &description() const { return _description_; }
    const QString &longDescription() const { return _longDescription_; }
};

#endif // ERROR_H
