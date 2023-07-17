#include "types.h"
#include "tools.h"

#include <iostream>
#include <iomanip>

/* alpha blend routine */
// https://www.daniweb.com/programming/software-development/code/216791/alpha-blend-algorithm
/*Need Tests. SERG - NOT USED IN PROJECT*/
//RGBA
//AlphaBlend(const unsigned int bg, const unsigned int src)
//{
//    const unsigned int a = src >> 24; /* alpha */
//    /* If source pixel is transparent, just return the background */
//    if(0 == a)
//        return {};
//    /* alpha blending the source and background colors */
//    const unsigned int rb
//        = (((src & 0x00ff00ff) * a) + ((bg & 0x00ff00ff) * (0xff - a))) & 0xff00ff00;
//    const unsigned int g
//        = (((src & 0x0000ff00) * a) + ((bg & 0x0000ff00) * (0xff - a))) & 0x00ff0000;
//    unsigned int rez = (src & 0xff000000) | ((rb | g) >> 8);
//    return *reinterpret_cast<RGBA *>(&rez);
//}

std::string
RGBA::toString() const
{
    std::stringstream stream;
    stream << "#";
    stream << std::setfill('0') << std::setw(2) << std::hex << int(r);
    stream << std::setfill('0') << std::setw(2) << std::hex << int(g);
    stream << std::setfill('0') << std::setw(2) << std::hex << int(b);
    stream << std::setfill('0') << std::setw(2) << std::hex << int(a);
    return stream.str();
}

RGBA
RGBA::make(const std::string & hex)
{
    try {
        return {uint8_t(std::stoi(hex.substr(1, 2), nullptr, 16)),
            uint8_t(std::stoi(hex.substr(3, 2), nullptr, 16)),
            uint8_t(std::stoi(hex.substr(5, 2), nullptr, 16)),
            uint8_t((hex.length() >= 9) ? std::stoi(hex.substr(7, 2), nullptr, 16) : 255)};
    } catch(const std::exception &) {
        throw std::invalid_argument(STR("Bad color:" + hex));
    }
}

RGBA
RGBA::makeBlend(const RGBA & c1, const RGBA & c2)
{
    return {uint8_t(c1.r + c2.a * (c2.r - c1.r) / 255), uint8_t(c1.g + c2.a * (c2.g - c1.g) / 255),
        uint8_t(c1.b + c2.a * (c2.b - c1.b) / 255), uint8_t(c1.a + c2.a * (255 - c1.a) / 255)};
}

RGBA
RGBA::makeBlendLinear(const RGBA & c1, const RGBA & c2, const float t)
{
    return {uint8_t(c1.r + t * (c2.r - c1.r)), uint8_t(c1.g + t * (c2.g - c1.g)),
        uint8_t(c1.b + t * (c2.b - c1.b)), uint8_t(c1.a + t * (c2.a - c1.a))};
}

ColorLevels
ColorLevels::make(
    const RGBA & under, const RGBA & over, const float min, const float max, const size_t steps)
{
    /// steps - это количество интервалов внутри min и max
    /// += 2 потому что добавляем интервалы (-inf, min] и (max, +inf)
    /// 1. (-inf, min] -> color under_
    /// 2. (min, max] - делим на steps количество шагов
    /// 3. (max, +inf) -> color over_
    ///
    /// в res.levels последнее значение больше чем max, это помогает
    /// при построении CLUT. Потому что res.levels.back() дает цвет для
    /// интервала слева (max, res.levels.back()] и
    /// справа (res.levels.back(), +inf) -> Как раз получается, что для
    /// всего интервала (max, +inf) идет один цвет = over_

    // инициализация res вектором размера steps и under, over
    ColorLevels res;
    res.resize(steps + 2);
    res.under = under;
    res.over = over;

    const float add = (max - min) / steps;
    float cur = min - add; // ибо (cur += add) сначала увеличит потом отдает

    // сгенерировать steps количество уровней.
    for(auto & i : res)
        i = (cur += add);

    return res;
}
