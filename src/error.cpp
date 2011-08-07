#include "error.h"

Error::Error(const QString &description, const QString &longDescription)
    : std::exception(),
      _description_(description),
      _longDescription_(longDescription)
{
}
