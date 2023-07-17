#ifndef CLUT_H
#define CLUT_H

#include "types.h"
#include <algorithm>
#include <stddef.h>

#define CLUT_SIZE 1000

class CLUT
{
private:
    float style_units_min; // in style units
    float style_units_dif;

    mutable float data_units_min; // in field variable units
    mutable float data_units_dif;

    vRGBA colors;
    vRGBA colorsI;

private:
    size_t dataToIndex(const float d) const
    {
        /// Get clut index using dataField units
        const float t = std::clamp((d - data_units_min) / data_units_dif, 0.0f, 1.0f);
        // while "t" is always [0, 1], the "index" is inside [0, colors.size() - 1]
        return size_t(t * (CLUT_SIZE - 1));
    }
    RGBA interpolated(const float d) const
    {
        /// Get single interpolated color using dataField units
        return colorsI[dataToIndex(d)];
    }
    RGBA flat(const float d) const
    {
        /// Get single flat color using dataField units
        return colors[dataToIndex(d)];
    }

public:
    static CLUT make(const Style & style);

public:
    const vRGBA & operator()(bool f) const
    {
        return f ? colors : colorsI;
    }

    vRGBA interpolatedPicture(const vData & data) const;
    vRGBA flatPicture(const vData & data) const;
    void setupUnits(const UnitConverter & converter) const;
};

#endif // CLUT_H
