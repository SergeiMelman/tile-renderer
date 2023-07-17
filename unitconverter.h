#ifndef UNITCONVERTER_H
#define UNITCONVERTER_H

#include <udunits2.h>
#include <memory>
#include <functional>

class UnitConverter
{
private:
    std::shared_ptr<ut_unit> unit1;
    std::shared_ptr<ut_unit> unit2;

    std::shared_ptr<cv_converter> converterF;
    std::shared_ptr<cv_converter> converterB;

public:
    static UnitConverter make(const char * const u1, const char * const u2);
    
    operator bool(){return bool(converterF);}

    float forward(float c) const;
    float backward(float c) const;
};

extern const UnitConverter K_degC;
extern const UnitConverter ms_knots;

#endif // UNITCONVERTER_H
