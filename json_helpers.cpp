#include "json_helpers.h"
#include "types.h"
#include "style.h"
#include "tools.h"
#include "dataquery.h"

#include <string>
#include <iostream>

// RGBA <-> json
void
from_json(const json & j, RGBA & c)
{
    c = RGBA::make(j.get<std::string>());
}

// Pair <-> json
void
from_json(const json & j, Pair & p)
{
    p = {j[0], RGBA::make(j[1])};
}

// Style <-> json
void
from_json(const json & j, Style & style)
{
    style = {};

    const auto it = j.find(Style::s_styles);
    if(it != j.end()) {
        it->get_to(style.styles);
    } else {
        j[Style::s_unit].get_to(style.units);
        j[Style::s_fill].get_to(style.fill);
        j[Style::s_isolines_style].get_to(style.isolines_style);
        j[Style::s_vector_type].get_to(style.vector_type);
        j[Style::s_vector_style].get_to(style.vector_style);
        const auto & jl = j[Style::s_levels];
        if(jl.size() == 3 && jl.back().is_number()) {
            // interpret as ColorLevels
            const auto & ju = jl[0]; // under & min
            const auto & jo = jl[1]; // over & max
            const ColorLevels & cl = ColorLevels::make(ju[1], jo[1], ju[0], jo[0], jl.back());
            style.levels = ColorPairsMake(cl);
        } else
            j[Style::s_levels].get_to(style.levels);

        // inerpret enums
        using ES = Style::EStyle;
        using EF = Style::EFill;
        using ET = Style::EType;
        const static std::map<std::string, ES> SESTYLE
            = {{"none", ES::none}, {"data", ES::data}, {"auto", ES::autos}};
        const static std::map<std::string, EF> SEFILL
            = {{"none", EF::none}, {"flat", EF::flat}, {"interpolated", EF::interpolated}};
        const static std::map<std::string, ET> SETYPE
            = {{"none", ET::none}, {"streamlines", ET::streamlines},
                {"arrowsdefault", ET::arrowsdefault}, {"", ET::fontbased}};

        try {
            style.isolines_style_enum = SESTYLE.at(style.isolines_style);
        } catch(const std::out_of_range &) {
            style.isolines_style_enum = ES::flat;
            style.isolines_style_color = RGBA::make(style.isolines_style);
        }

        try {
            style.vector_style_enum = SESTYLE.at(style.vector_style);
        } catch(const std::out_of_range &) {
            style.vector_style_enum = ES::flat;
            style.vector_style_color = RGBA::make(style.vector_style);
        }

        style.fill_enum = SEFILL.at(style.fill);
        try {
            style.vector_type_enum = SETYPE.at(style.vector_type);
        } catch(const std::out_of_range &) {
            style.vector_type_enum = ET::fontbased; // Becouse int the style.vector_type we have FontName
        }

        const std::string & lt = j[Style::s_triangle_left];
        if((style.triangle_left = (lt != "none")))
            style.triangle_left_color = style.levels.front().color;

        const std::string & rt = j[Style::s_triangle_right];
        if((style.triangle_right = (rt != "none")))
            style.triangle_right_color = style.levels.back().color;

        style.clut = CLUT::make(style);
    }
}

// DataQuery <-> json
void
from_json(const json & j, DataQuery & dataQ)
{
    dataQ = {};
    dataQ.file = j[D_file];
    j[D_variables].get_to(dataQ.variables);
    dataQ.style = j[D_style];
    dataQ.units = j[D_units];
    dataQ.time = j[D_time];
    dataQ.XYZoom.x = j[D_x];
    dataQ.XYZoom.y = j[D_y];
    dataQ.XYZoom.zoom = j[D_zoom];
}

// Fill a style using its ancestors
const json &
getFullStyleJson(json & style, json & styles)
{
    const json::iterator it = style.find(Style::s_parent);
    if(it != style.end()) { // If style has an ancestor
        const std::string parent_name = it->get<std::string>();
        style.erase(it); // We don't need it more
        const json patch = style;
        // Get a fully filled ancestor
        const json::iterator parent = styles.find(parent_name);
        if(parent != styles.end()) {
            style = getFullStyleJson(*parent, styles);
            style.merge_patch(patch); // On an ancestor we apply a child's data
        } else {
            const auto err = STR("There is no parent style: " + parent_name);
            std::cout << err << std::endl;
            throw std::out_of_range(err);
        }
    }
    return style;
}

// Fill all styles using inheritance and ancestors
const json &
fillStylesJson(json & j_styles)
{
    for(auto & s : j_styles)
        getFullStyleJson(s, j_styles);

    return j_styles;
}
