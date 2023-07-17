#include "tools.h"
extern const char * const j_str = R"delim(
{
  "data": [
    {
      "file1": "/data/wxtiles/gfs20190214_12z.nc",
      "file": "/data/wxtiles/gfs20190213_18z.nc",

      "style": "base_barbs",
      "variables": ["UGRD_10maboveground", "VGRD_10maboveground"],
      
      "style2": "multilevel_style",
      "variables2": ["TMP_2maboveground"],

      "time": 16
    }
  ],
  "styles": {

    "base_barbs": {
      "parent": "base",
      "levels": [
[25.0,"#ff00ffff"],
[40.0,"#0000ffff"],
[50.0,"#ffff00ff"],
[60.0,"#00ff00ff"],
[70.0,"#ff00ffff"],
[80.0,"#00ffffff"]],

      "unit": "knots",

      "fill-NOTE": "flat, interpolated, none",
      "fill": "interpolated",

      "isolines_style-NOTE":"none, #bbaa88ff - flat color, data, auto",
      "isolines_style": "auto",

      "vector_style-NOTE": "none, #bbaa88ff - flat color, data, auto",
      "vector_style": "auto",

      "vector_type-NOTE": "none, streamlines, arrowsdefault - or fontbased filename.ttf",
      "vector_type": "streamlines"

    },
    "base_arrows1": {
      "parent": "base",
      "levels1": [[0.0,"#ff00ffff"],[10.0,"#0000ffff"],[20.0,"#00ff00ff"],[30.0,"#00ffffff"]],
      "levels": [[0.0,"#ff0000ff"],[60.0,"#00f000ff"], 8],
      "unit": "knots",

      "fill-NOTE": "flat, interpolated, none",
      "fill": "flat",

      "isolines_style-NOTE":"none, #bbaa88ff - flat color, data, auto",
      "isolines_style": "ffffffff",

      "vector_style-NOTE": "none, #bbaa88ff - flat color, data, auto",
      "vector_style": "#ff00ffff",

      "vector_type-NOTE": "none, streamlines, arrowsdefault - or fontbased filename.ttf",
      "vector_type": "arrows1"

    },

    "base": {
      "fill-NOTE": "flat, interpolated, none",
      "fill": "interpolated",

      "isolines_style-NOTE":"none, #bbaa88ff - flat color, data, auto",
      "isolines_style": "auto",

      "vector_style-NOTE": "none, #bbaa88ff - flat color, data, auto",
      "vector_style": "none",

      "vector_type-NOTE": "none, barbs, streamlines, arrows1, arrows2, arrows3",
      "vector_type": "none",

      "leextern const char * const j_strvels-NOTE": "ColorPares (level, color),  ColorLevels (pare, pare, levels) ",
      "levels": [[-18.0,"#000000FF"],[36.0,"#fefefeff"], 10],

      "unit": "degC",

      "triangle-NOTE": "for legend rendering purpose only: none - no triangle, auto - from levels",
      "triangle_left": "auto",
      "triangle_right": "none"
    },


    "St1": {
      "levels": [[-50.0,"#00000FF"],[36.0,"#ffffffff"], 10],
      "parent": "base"
    },
    "St2": {
      "levels": [[-50.0,"#ff0000FF"],[36.0,"#00ffffff"], 20],
      "parent": "base"
    },
    "St3": {
      "levels": [[-50.0,"#0000ffFF"],[36.0,"#00ff00ff"], 40],
      "parent": "base"
    },
    "multilevel_style": {
      "styles": [[3,"St1"],[8,"St2"],[12,"St3"]]
    }
  }
}
)delim";

// reference 58.png
// v1/wxtiles/tile/ncep-gfs-global-wind10m/wind-speed-direction-filled/20190213_18/2019-02-14T00:00:00Z/0/7/122/58.png

// reference 116.png
// https://api.wxtiles.com/v1/wxtiles/tile/ncep-gfs-global-wind10m/wind-speed-direction-filled-barbs/20190214_12/2019-02-16T12:00:00Z/0/8/244/116.png?apikey=gregcisace

#include "style.h"
#include "dataquery.h"
#include "font.h"
#include "unitconverter.h"
#include "mercantile.h"

#include <regex>
#include <iostream>
#include <sstream>
#include <iomanip>

////
/// \brief splitUrl splits the Url into words. "Word" is a set of any symbols except '/' and '.'
std::vector<std::string>
splitUrl(const std::string & url)
{
    const std::regex word_regex("[^/.]+"); // split delimiters '/' and '.'
    std::vector<std::string> vs;
    std::sregex_iterator start{url.begin(), url.end(), word_regex};
    const std::sregex_iterator end;
    while(start != end)
        vs.push_back(start++->str());

    return vs;
}

////
/// \brief getStyleNameFromMap - selects a style from a styles set according to data Query
/// and multy style.
/// \param styles
/// \param dataQ
/// \return - style name
std::string
getStyleNameFromMap(const StylesMap & styles, const DataQuery & dataQ)
{
    if(styles.empty())
        throw std::out_of_range(STR("Styles are empty."));

    const auto data_style_it = styles.find(dataQ.style);

    if(data_style_it == styles.end())
        throw std::out_of_range(STR("Cannot find style: " + dataQ.style));

    const Style & proxi_style = data_style_it->second;
    if(proxi_style.styles.empty()) {
        // style is not multizoomlevel
        return dataQ.style;
    }

    // find the appropriate style from multystyle for zoom level
    auto mstyles_it = proxi_style.styles.lower_bound /*upper_bound*/ (dataQ.XYZoom.zoom);
    if(mstyles_it == proxi_style.styles.end())
        --mstyles_it;
    const std::string & style_for_rendering_name = mstyles_it->second;
    auto style_for_rendering_it = styles.find(style_for_rendering_name);
    if(style_for_rendering_it == styles.end())
        throw std::out_of_range(
            STR("No style: " + style_for_rendering_name + ", in multystyle: " + dataQ.style));

    return style_for_rendering_name;
}

////
/// \brief ColorPairsMake - make ColorPairs from ColorLevels
ColorPairs
ColorPairsMake(const ColorLevels & levels)
{
    ColorPairs res;
    const float min = levels.front();
    const float max = levels.back();
    const float dif = max - min;

    //
    res.push_back({levels.front(), levels.under});
    const float t1 = (*(levels.end() - 1) - *(levels.end() - 2)) / dif;
    const RGBA s_color = RGBA::makeBlendLinear(levels.under, levels.over, t1);

    for(auto i : levels) {
        const float t = (i - min) / dif;
        res.push_back({i, RGBA::makeBlendLinear(s_color, levels.over, t)});
    }
    // res.push_back(res.back());
    return res;
}

////
/// \brief printd - imprint float into a picture at x, y
void
printd(float d, RGBA c, Font & font, vRGBA & picture, int x, int y)
{
    std::wostringstream ss;
    ss << std::fixed << std::setprecision(2) << d;
    print(ss.str(), c, font, picture, x, y);
}

////
/// \brief drawBitmap - imprint fonts glyph's bitmap into picture
/// \param ppic - pointer to the first pixel for imprinting
/// \param bmp - font glyph's bitmap
/// \param color - just a color of symbol
void
drawBitmap(RGBA * ppic, size_t picwidth, const FT_Bitmap * bmp, const RGBA c)
{
    const uint8_t * pbuf = bmp->buffer;
    // draw bitmap
    for(size_t j = bmp->rows; j--; ppic += picwidth - bmp->width)
        for(size_t i = bmp->width; i--; ++pbuf, ++ppic)
            if(*pbuf) // skip lots of zeros
                ppic->blend((*pbuf < 100) ? RGBA{0, 0, 0, uint8_t(c.a * *pbuf / 255)}
                                          : RGBA{c.r, c.g, c.b, uint8_t(c.a * *pbuf / 255)});
}

////
/// \brief print - imprint color string into picture at position x, y using font
void
print(const std::wstring & str, RGBA color, Font & font, vRGBA & picture, int x, int y)
{
    const Font::Glyphs glyphs = font.makeString(str);
    x -= (glyphs.box.xMax - glyphs.box.xMin) / 2; // shift to text box center
    y += (glyphs.box.yMax - glyphs.box.yMin) / 2;

    const int j_start = y - glyphs.box.yMax - 1;
    const int j_end = y - glyphs.box.yMin + 1;
    const int i_start = x + glyphs.box.xMin - 1;
    const int i_end = x + glyphs.box.xMax + 1;

    if(j_start < 0 || j_end >= int(picture.getHeight()) || i_start < 0 || i_end >= int(picture.getWidth()))
        return;

    const int di = i_end - i_start;
    const int dj = j_end - j_start;
    RGBA * ppic = &picture(i_start, j_start);

    for(int j = dj; j--; ppic += picture.getWidth() - di)
        for(int i = di; i--; ++ppic)
            ppic->a = 1; // text background mark

    for(const auto gl : glyphs.glyphs)
        drawBitmap(&picture(x + gl->left, y - gl->top), picture.getWidth(), &gl->bitmap, color);
}

////
/// \brief printVectorXY - used for vector field rendering. Imprint one vector font simbol
/// \param tdata - all tile data variable
/// \param converter - unit converrter field<->style units
/// \param color - color
/// \param font - specially constructed font vith symbols of 'vector'
/// \param picture - picture to render into
/// \param x - position
/// \param y - position
void
printVectorXY(const TileData & tdata, const UnitConverter & converter, const RGBA color,
    Font & font, vRGBA & picture, size_t x, size_t y)
{
    const vData & U = tdata.data[0];
    const vData & V = tdata.data[1];
    const vData & R = tdata.data[2];
    const float r = R(x, y);
    /* set up rotation matrix */
    const auto cs = FT_Fixed(V(x, y) / r * 0x10000L);
    const auto sn = FT_Fixed(-U(x, y) / r * 0x10000L);
    FT_Matrix matrix = {cs, -sn, sn, cs};
    // get the barb simbol's number using converting to knots. 145 knots - maximun speed to render
    const FT_ULong ch = size_t(std::min(converter.backward(r), 145.0f)) / 5 + 'A';
    // make Vectors's picture
    FT_BitmapGlyph gl = font.makeGlyph(ch, &matrix, nullptr);
    /// there is NO 'out of bounds' test, hoping for proper set up of font's size, etc.
    RGBA * ppic = &picture(x + gl->left, y - gl->top);
    drawBitmap(ppic, picture.getWidth(), &gl->bitmap, color);
    FT_Done_Glyph(FT_Glyph(gl));
}
