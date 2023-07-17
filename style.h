#ifndef STYLE_H
#define STYLE_H

#include "types.h"
#include "clut.h"

#include <map>
#include <string>

struct Style
{
    enum class EStyle {flat, data, autos, none};
    enum class EFill {flat, interpolated, none};
    enum class EType {streamlines, arrowsdefault, fontbased, none};
    
    static constexpr char s_parent[]{"parent"};

    static constexpr char s_styles[]{"styles"};
    std::map<size_t, std::string> styles;
    
    static constexpr char s_unit[]{"unit"};
    std::string units;

    static constexpr char s_triangle_left[]{"triangle_left"};
    bool triangle_left{};
    RGBA triangle_left_color{};
    static constexpr char s_triangle_right[]{"triangle_right"};
    bool triangle_right{};
    RGBA triangle_right_color{};

    static constexpr char s_fill[]{"fill"};
    std::string fill; //(flat, interpolated, none)
    EFill fill_enum;

    static constexpr char s_isolines_style[]{"isolines_style"};
    std::string isolines_style; // 'none', '#bbaa88ff' - flat color, 'data', 'auto'
    RGBA isolines_style_color{255,255,255,255}; // essential to be not zero
    EStyle isolines_style_enum;

    static constexpr char s_vector_type[]{"vector_type"};
    std::string vector_type; //(none, barbs, streamlines, arrows1, arrows2, arrows3...)
    EType vector_type_enum;
    static constexpr char s_vector_style[]{"vector_style"};
    std::string vector_style; // 'none', '#bbaa88ff' - flat color, 'data', 'auto'
    RGBA vector_style_color;
    EStyle vector_style_enum;
    
    static constexpr char s_levels[]{"levels"};
    ColorPairs levels;
    
    CLUT clut;
};

#endif // STYLE_H
