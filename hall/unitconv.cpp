#include "unitconv.h"


/*UnitConv::UnitConv()
{
}*/

double UnitConv::fromDisplay(double val, const DisplayUnit_t &unit)
{
    return val / unit.scale;
}

double UnitConv::toDisplay(double val, const DisplayUnit_t &unit)
{
    return val * unit.scale;
}
