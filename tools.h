#ifndef TOOLS_H
#define TOOLS_H

#include <vector>
#include <string>

#include "types.h"

extern const char * const j_str;

std::vector<std::string> splitUrl(const std::string & url);
// It selects a style from a styles set according to data Query
// and multy style.
std::string getStyleNameFromMap(const StylesMap & styles, const DataQuery & dataQ);

ColorPairs ColorPairsMake(const ColorLevels & levels);

class Font;
void printd(float d, RGBA c, Font & font, vRGBA & picture, int x, int y);
void print(const std::wstring & str, RGBA color, Font & font, vRGBA & picture, int x, int y);
void printVectorXY(const TileData & tdata, const UnitConverter & converter, const RGBA color,
    Font & font, vRGBA & picture, size_t x, size_t y);

#define COUT_EXCEPTION                                                                       \
    {                                                                                        \
        std::cout << __FILE__ << ":" << __func__ << ":exception:" << ex.what() << std::endl; \
    };

#define COUT_INFO(_X_)                                                       \
    {                                                                        \
        std::cout << __FILE__ << ":" << __func__ << ":" << _X_ << std::endl; \
    };

#define STR(_X_) std::string(&__FILE__[0]) + ':' + &__func__[0] + ':' + _X_

#endif // TOOLS_H
