#include "grody/webserver.h"
#include "clientdatacache.h"
#include "pngwrap.h"
#include "tools.h"
#include "json_helpers.h"
#include "dataquery.h"
#include "style.h"
#include "font.h"

#include <iostream>

#define LEGEND_W 380
#define LEGEND_H 85

// TODO: finish it!
void
legend_handler(struct client * client) try {
    // const auto data = pClientDataCache(client->data);
    // const auto vs = splitUrl(client->req.url);
    
    std::cout << "Legend rendering!" << std::endl;    
    
    const std::string body = j_str; // client->req.body;

    nlohmann::json j = json::parse(body);
    const StylesMap stylesMap = fillStylesJson(j["styles"]);
    const std::vector<DataQuery> datav = j["data"];
    const DataQuery & dataQ = datav.front();
    const Style & style = stylesMap.at(getStyleNameFromMap(stylesMap, dataQ));
    const CLUT & clut = style.clut;

    constexpr size_t border = 4;
    constexpr size_t vert_band_size = 30;
    constexpr size_t left_pos = vert_band_size / 2 + border;
    constexpr size_t right_pos = LEGEND_W - left_pos;
    constexpr size_t upper_pos = border;
    constexpr size_t bottom_pos = upper_pos + vert_band_size;

    vRGBA legend(LEGEND_W, LEGEND_H);
    const vRGBA & c_colors = clut(style.fill_enum == Style::EFill::flat);
    
    // draw color band
    for(size_t x = left_pos; x < right_pos; ++x) {
        const size_t clut_pos = ((x - left_pos) * CLUT_SIZE) / (right_pos - left_pos);
        const RGBA pos_color = c_colors[clut_pos];
        for(size_t y = upper_pos; y < bottom_pos; ++y) {
            legend(x, y) = pos_color;
        }
    }

    // Draw left and right triangles
    for(size_t t = 0; t < vert_band_size / 2; ++t) {
        for(size_t y = upper_pos + t; y < bottom_pos - t; ++y) {
            if(style.triangle_left) {
                legend(left_pos - t, y) = style.triangle_left_color; // c_colors.front();
            }
            if(style.triangle_right) {
                legend(right_pos + t, y) = style.triangle_right_color; // c_colors.back();
            }
        }
    }
    
    // print tics and vars
    const auto min_pair = style.levels.begin();
    const auto max_pair = style.levels.end() - (style.triangle_right ? 2 : 1);
    const float min_d = min_pair->d;
    const float max_d = max_pair->d;
    const float dif_d = max_d - min_d;
    int i = 0;
    Font font = Font::make("vera.ttf", 11); // NOTE: Could be static???
    
    for(auto c = min_pair; c <= max_pair; ++c) {
        // calc pos
        const size_t x = left_pos + size_t((right_pos - left_pos) * (c->d - min_d) / dif_d);
        for(size_t y = bottom_pos - 3; y < bottom_pos + 3; ++y) {
            legend(x, y) = {255, 255, 0, 255};
        }
        printd(c->d, c->color, font, legend, x, bottom_pos + 8 + 0 * 10);
        ++i;
    }

    Font font2 = Font::make("vera.ttf", 16); // NOTE: Could be static???
    const std::wstring wsTmp(style.units.begin(), style.units.end());
    print(wsTmp, {0,0,0,255}, font2, legend, LEGEND_W / 2, bottom_pos + 25);
    
    const PNGWrap png(legend);
    const size_t size = png.getBufLength();

    if(start_response(client, http_ok, http_ok_text, mime_png, size))
        return;

    client_send(client, png.getBufBytes(), size);
} catch(const std::exception & ex) {
    COUT_EXCEPTION;
}
