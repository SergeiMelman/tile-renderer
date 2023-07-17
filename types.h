#ifndef TYPES_H
#define TYPES_H

#include <vector>
#include <string>
#include <cstdint>
#include <map>

using std::size_t;

struct Style;
struct DataQuery;
class DataFile;

template<class T>
class vectorEx : public std::vector<T>
{
    size_t width;

public:
    vectorEx(size_t w = 0, size_t h = 1, T def = {})
        : std::vector<T>(w * h, def)
        , width(w)
    {
    }

    size_t getWidth() const
    {
        return width;
    }

    size_t getHeight() const
    {
        return this->size() / width;
    }

    void set(size_t w, size_t h)
    {
        width = w;
        this->resize(w * h);
    }

    const T & operator()(size_t x, size_t y) const
    {
        return (*this)[y * width + x];
    }

    T & operator()(size_t x, size_t y)
    {
        return (*this)[y * width + x];
    }
};

class RGBA
{
public:
    uint8_t r, g, b, a;

public:
    inline operator uint32_t() const;
    inline void blend(const RGBA & c2);
    inline void blendFast(const RGBA & c2);
    inline void blendA255(const RGBA & c2);
    inline RGBA operator~() const;

    std::string toString() const;

    static RGBA make(const std::string & hex);
    static RGBA makeBlend(const RGBA & c1, const RGBA & c2);
    static RGBA makeBlendLinear(const RGBA & c1, const RGBA & c2, const float t);
};

inline RGBA::operator uint32_t() const
{
    return *(reinterpret_cast<const uint32_t *>(&r));
}

void
RGBA::blend(const RGBA & c2)
{
    const int outA = 255 * c2.a + a * (255 - c2.a);
    if(outA == 0) {
        *this = {};
    } else {
        const int preA = a * (255 - c2.a) / 255;
        *this = {uint8_t(255 * (c2.r * c2.a + r * preA) / outA),
            uint8_t(255 * (c2.g * c2.a + g * preA) / outA),
            uint8_t(255 * (c2.b * c2.a + b * preA) / outA), uint8_t(outA / 255)};
    }
}

inline void
RGBA::blendFast(const RGBA & c2)
{
    r += (c2.a * (c2.r - r) / 255);
    g += (c2.a * (c2.g - g) / 255);
    b += (c2.a * (c2.b - b) / 255);
    a += (c2.a * (255 - a) / 255);
}

inline void
RGBA::blendA255(const RGBA & c2)
{
    r += (c2.a * (c2.r - r) / 255);
    g += (c2.a * (c2.g - g) / 255);
    b += (c2.a * (c2.b - b) / 255);
    a = 255;
}

inline RGBA
RGBA::operator~() const
{
    return {uint8_t(~r), uint8_t(~g), uint8_t(~b), a};
}

struct Vec2D
{
    float x, y;
};

struct Pair
{
    float d;
    RGBA color;
};

using vDouble = std::vector<double>;
using vString = std::vector<std::string>;
using vRGBA = vectorEx<RGBA>;
using vData = vectorEx<float>;
using vvData = std::vector<vData>;
using ColorPairs = std::vector<Pair>;
using StylesMap = std::map<std::string, Style>;
using FilesMap = std::map<std::string, DataFile>;

struct LonLat
{
    vDouble lon; // X
    vDouble lat; // Y
};

struct VariableData
{
    vvData data;
    std::string units;
};

using TileData = VariableData; // the same content - just different name

using VariablesMap = std::map<std::string, VariableData>;

class ColorLevels : public std::vector<float>
{
public:
    RGBA under, over;
    static ColorLevels make(const RGBA & under_, const RGBA & over_, const float min,
        const float max, const size_t steps);
};

class UnitConverter;
using UnitConvertersMap = std::map<std::pair<std::string, std::string>, UnitConverter>;

#endif // TYPES_H
