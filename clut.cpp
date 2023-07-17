#include "clut.h"
#include "style.h"
#include "unitconverter.h"
#include "tools.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
CLUT
CLUT::make(const Style & style)
{
    if(style.levels.empty())
        throw std::invalid_argument(STR("Levels are empty in style."));

    CLUT clut;
    // On this stage units convertion does not matter
    clut.style_units_min = style.levels.front().d;
    clut.style_units_dif = style.levels.back().d - clut.style_units_min;

    clut.data_units_min = clut.style_units_min;
    clut.data_units_dif = clut.style_units_dif;

    clut.colors.resize(CLUT_SIZE);
    clut.colorsI.resize(CLUT_SIZE);

    // +1 because the first pair is used already
    auto tec_pair_it = style.levels.cbegin();

    // for NON-interpolated color
    auto tec_color_it = clut.colors.begin();
    *tec_color_it++ = tec_pair_it->color;

    auto const last_color_it = clut.colors.end() - 1;
    *last_color_it = style.levels.back().color;

    // for interpolated color
    auto tec_colorI_it = clut.colorsI.begin();
    *tec_colorI_it++ = tec_pair_it->color;
    auto const last_colorI_it = clut.colorsI.end() - 1;
    *last_colorI_it = style.levels.back().color;

    const float step_size = clut.style_units_dif / CLUT_SIZE;
    float current_data_pos = clut.style_units_min;

    ++tec_pair_it;

    while(tec_color_it != last_color_it) {
        current_data_pos += step_size;
        // CLUT_SIZE should be big enought, and "dt" small.
        // Any two close colors positions in "color_pairs" should be bigger then "add"
        // Or else some color positions could be dropped, not a big issue though
        while(tec_pair_it->d < current_data_pos)
            ++tec_pair_it; // should never overflow

        // non interpolated
        *tec_color_it++ = tec_pair_it->color;

        // interpolated
        const float lmin = (tec_pair_it - 1)->d;
        const float lmax = (tec_pair_it + 0)->d;
        const float lt = (current_data_pos - lmin) / (lmax - lmin);
        *tec_colorI_it++
            = RGBA::makeBlendLinear((tec_pair_it - 1)->color, (tec_pair_it + 0)->color, lt);
    } // while

    return clut;
}

/// Get interpolated picture using data
vRGBA
CLUT::interpolatedPicture(const vData & data) const
{
    vRGBA picture(data.getWidth(), data.getHeight());
    auto p = picture.begin();
    for(auto d : data)
        *p++ = interpolated(d);
    return picture;
}

/// Get flat picture using interpolated data
vRGBA
CLUT::flatPicture(const vData & data) const
{
    vRGBA picture(data.getWidth(), data.getHeight());
    auto p = picture.begin();
    for(auto d : data)
        *p++ = flat(d);
    return picture;
}

/// For using with dataToIndex to convert fieldData to clut index
void
CLUT::setupUnits(const UnitConverter & converter) const
{
    data_units_min = converter.forward(style_units_min);
    data_units_dif = converter.forward(style_units_dif + style_units_min) - data_units_min;
}
