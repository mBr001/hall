#ifndef UNITCONV_H
#define UNITCONV_H

typedef struct DisplayUnit {
    const double scale;
    const char* unitText;
    const char* unitHtml;
} DisplayUnit_t;

const DisplayUnit_t carriercUnits = {
    1e6, "cm^-3", "<span>cm<sup>-3</sup></span>"
};

const DisplayUnit_t hallProbeIUnits = {
    1e-3, "mA", "mA"
};

const DisplayUnit_t RHallUnits = {
    1e-6, "cm^3·C^-1", "<span>cm<sup>3</sup>C<sup>-1</sup></span>"
};

const DisplayUnit_t driftUnits = {
    1e-4, "cm^2·V^-1·s^-1", "<span>cm<sup>2</sup>V<sup>-1</sup>s<sup>-1</sup></span>"
};

const DisplayUnit_t resistivitySpecUnits = {
    1e-2, "Ω·cm", "Ω·cm"
};

const DisplayUnit_t sampleIUnits = {
    1e-3, "mA", "mA"
};

const DisplayUnit_t sampleThicknessUnits = {
    1e-6, "μm", "μm"
};

class UnitConv
{
    UnitConv();

public:
    static double fromDisplay(double val, const DisplayUnit_t &unit);
    static double toDisplay(double val, const DisplayUnit_t &unit);
};

#endif // UNITCONV_H
