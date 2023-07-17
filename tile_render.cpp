#include "grody/webserver.h"
#include "clientdatacache.h"
#include "pngwrap.h"
#include "tools.h"
#include "dataquery.h"
#include "style.h"
#include "datafile.h"
#include "nlohmann/json.hpp"
#include "tile.h"
#include "clut.h"
#include "mercantile.h"
#include "font.h"
#include "drawprimitives.hpp"

#include <algorithm>

void render(
    vRGBA & picture, const Style & style, const TileData & dataEx, const UnitConverter & converter);

// TODO: test time for rendering without cache

void
tile_handler(struct client * client) try {
    auto start = std::chrono::system_clock::now();
    // Parse incomings
    // Get DataQuery
    // Get File using filename (cached)
    // Get Style(CLUT) using DataQuery (cached) (Думаю над этим пунктом)
    // Get Lonlat using File (cached)
    // Get VariableLayer using File and Time (cached)
    // Get Tile using (tx,ty,zoom) and Lolat
    // Get Data using Tile and VariableLayer
    // Get Picture using CLUT and Data
    // Get PNG using Picture
    // client_send PNG

    // helpfull variables
    const auto clientDataCache = pClientDataCache(client->data);
    //const std::string url = client->req.url;
    //client->req.body;
    //client->req.body_size;
    //const auto surl = splitUrl(url);

    const std::string body = client->req.body;
    std::cout << body << std::endl;

    // Parse incomings
    // in URL
    json j = json::parse(body);
    clientDataCache->set(body, j);
    // Get DataQuery
    std::vector<DataQuery> datav = j["Data"]; // j["data"].get_to(datav);

    vRGBA picture(TILE_SIZE, TILE_SIZE);

    for(DataQuery & dataQ : datav) {
        // Get File using filename (cached)
        const DataFile & ncfile = clientDataCache->getFile(dataQ);

        // Get Style(CLUT) using DataQuery (cached)
        const Style & style = clientDataCache->getStyle(dataQ);

        // Get VariableLayer using DataQuery (cached)
        const VariableData & variableData = clientDataCache->getVariableData(dataQ);

        // Get Tile using (tx,ty,tzoom) and LonLat
        const TileCoords & tileCoords = TileCoords::make(ncfile.getLonLat(), dataQ.XYZoom);

        // Get DataEx (1 + TILE_SIZE + 1)^2 using Tile and variableData
        const TileData & dataEx = tileCoords.interpolate(variableData);

        const UnitConverter & converter = clientDataCache->getUnitConverter(style, variableData);

        // Render to Picture using alfablending, CLUT and Data
        render(picture, style, dataEx, converter);
    }

    // print(L"ABCDEFGHijklmnop. Hello world!!", {255, 255, 0, 255}, font, picture, 125, 125);

    // Get PNG using Picture
    const PNGWrap png(picture);

    // client_send PNG
    const size_t size = png.getBufLength();

    if(start_response(client, http_ok, http_ok_text, mime_png, size)) {
        throw std::invalid_argument(STR("URL: XYZoom parcing error."));
        std::cout << STR("Duck!\n");
        return;
    }

    client_send(client, png.getBufBytes(), size);

    std::chrono::duration<double> diff = std::chrono::system_clock::now() - start;
    std::cout << "Rendering time: " << diff.count() << std::endl;

} catch(const std::exception & ex) {
    COUT_EXCEPTION;
}

void renderVectorFont(vRGBA &, const Style &, const TileData &, const vRGBA &);
void renderArrows(vRGBA &, const Style &, const TileData &, const vRGBA &);
void renderStreamLines(vRGBA &, const Style &, const TileData &, const vRGBA &);
void renderNone(vRGBA &, const Style &, const TileData &, const vRGBA &){}

void
render(
    vRGBA & picture, const Style & style, const TileData & dataEx, const UnitConverter & converter)
{
    if(dataEx.data.empty())
        return;

    const CLUT & clut = style.clut;
    clut.setupUnits(converter);

    // the last of 'variablesData' is always 'scalar'.
    // because here is 'velocities' for the vector field .
    const auto & vdataEx = dataEx.data.back();
    const vRGBA & flatPicEx = clut.flatPicture(vdataEx); // Extended (1 + TILE_SIZE + 1)
    const vRGBA & interpolatedPicEx
        = clut.interpolatedPicture(vdataEx); // Extended (1 + TILE_SIZE + 1)

    /// --SCALAR-- flat\interpolated filling
    if(style.fill_enum != Style::EFill::none) {
        auto c = picture.begin();
        const vRGBA & pics
            = (style.fill_enum == Style::EFill::flat) ? flatPicEx : interpolatedPicEx;
        for(size_t y = 1; y < TILE_SIZE + 1; ++y)
            for(size_t x = 1; x < TILE_SIZE + 1; ++x, ++c)
                // c->blend(pics(x, y));
                c->blendFast(pics(x, y)); // NOTE: 2x faster but not true blend
    }

    // TODO: векторное поле не должно перечеркивать текст!!!!
    /// --VECTOR-- rendering
    //// alternative boasting! Men!!
    using renderFunc = decltype(&renderNone);
    renderFunc renderVector[]{renderStreamLines, renderArrows, renderVectorFont, renderNone};
    renderVector[int(style.vector_type_enum)](picture, style, dataEx, interpolatedPicEx);

    /// --ISOLINES--
    if(style.isolines_style_enum != Style::EStyle::none) {
        Font font = Font::make("vera.ttf", 11); // NOTE: Could be static???
        vRGBA isolinesPic(TILE_SIZE, TILE_SIZE);
        auto c = isolinesPic.begin();
        uint8_t _rnd_ = 0;
        // test every pixel, mark isoline and put text
        for(size_t y = 1; y < TILE_SIZE + 1; ++y) {
            for(size_t x = 1; x < TILE_SIZE + 1; ++x, ++c) {
                const RGBA tmpClr = flatPicEx(x, y);
                // if this pixel is empty and it is isoline...
                if(!c->a && (tmpClr != flatPicEx(x + 1, y) || tmpClr != flatPicEx(x, y + 1))) {
                    // upply the style/select the color.
                    switch(style.isolines_style_enum) {
                    case Style::EStyle::flat:
                        *c = style.isolines_style_color;
                        break;
                    case Style::EStyle::autos:
                        *c = ~interpolatedPicEx(x, y); // auto'modify' base color
                        break;
                    case Style::EStyle::data:
                        *c = interpolatedPicEx(x, y); // get base color from interpolated data
                        break;
                    case Style::EStyle::none:;
                    } // switch

                    // let's add text 'occasionally' - once at each 256's cycle
                    if(!++_rnd_)
                        printd(converter.backward(vdataEx(x, y)), *c, font, isolinesPic, x, y);

                } // if( isoline test
            } // for x
        } // for y

        /// blend two pictures!
        auto pSrc = isolinesPic.cbegin(); // source
        auto pDst = picture.begin(); // destination
        for(size_t i = picture.size(); i--; ++pSrc, ++pDst)
            if(pSrc->a) // skip zeros
                // pDst->blend(*pSrc);
                pDst->blendFast(*pSrc); // NOTE: 2x faster but not true blend

    } // if(style.eisolines_style)
}

inline RGBA
getVectorStyleColor(const Style & style, const vRGBA & interpolatedPicEx, size_t x, size_t y)
{
    // NOTE: Ignore _Ex :) - not a big deal
    switch(style.vector_style_enum) {
    case Style::EStyle::flat:
        return style.vector_style_color;
    case Style::EStyle::autos:
        return ~interpolatedPicEx(x, y); // auto'modify' base color
    case Style::EStyle::data: // get base color from interpolated data
        return interpolatedPicEx(x, y);
    case Style::EStyle::none:;
    }
    return {};
}

void
renderVectorFont(
    vRGBA & picture, const Style & style, const TileData & tdataEx, const vRGBA & interpolatedPicEx)
{
    if(tdataEx.data.size() != 3)
        return;

    const UnitConverter converter = UnitConverter::make("knots", tdataEx.units.c_str());

    constexpr int cell = 32; // in pixels
    Font vectorFont = Font::make(style.vector_type + ".ttf", 35);

    for(size_t y = cell / 2; y <= TILE_SIZE - cell / 2; y += cell) {
        for(size_t x = cell / 2; x <= TILE_SIZE - cell / 2; x += cell) {
            const RGBA color = getVectorStyleColor(style, interpolatedPicEx, x, y);
            printVectorXY(tdataEx, converter, color, vectorFont, picture, x, y); // NOTE: Ignore _Ex :) - not a big deal
        }
    }
}

void
renderArrows(
    vRGBA & picture, const Style & style, const TileData & tdataEx, const vRGBA & interpolatedPicEx)
{
    if(tdataEx.data.size() != 3)
        return;

    // convert data to knots - I use knots as standart units for vector rendering
    const UnitConverter converter = UnitConverter::make("knots", tdataEx.units.c_str());

    constexpr int cell = 32;

    const vData & U_Ex = tdataEx.data[0];
    const vData & V_Ex = tdataEx.data[1];
    const vData & dataEx = tdataEx.data[2];

    for(size_t y = cell / 2; y <= TILE_SIZE - cell / 2; y += cell) {
        for(size_t x = cell / 2; x <= TILE_SIZE - cell / 2; x += cell) {
            const RGBA color = getVectorStyleColor(style, interpolatedPicEx, x, y);

            const float r = dataEx(x + 1, y + 1);  // +1 Becouse of _Ex
            const float v = U_Ex(x + 1, y + 1) / r; // +1 Becouse of _Ex
            const float u = -V_Ex(x + 1, y + 1) / r; // +1 Becouse of _Ex
            // Calc multiplier using converting to knots. 145 knots is maximum speed to render.
            const float muld = (cell / (145.0f * 2.5f)) * std::min(converter.backward(r), 145.0f);

            const float mx = muld * v;
            const float my = muld * u;

            const auto x1 = size_t(x - mx);
            const auto y1 = size_t(y - my);
            const auto x2 = size_t(x + mx);
            const auto y2 = size_t(y + my);

            // point base
            circle(picture, color, x2, y2, 1);
            wuline(picture, color, int(x1), int(y1), int(x2), int(y2));
            // plotLineWidth(picture, color, int(x1), int(y1), int(x2), int(y2), 1.5);
        }
    }
}

void
renderStreamLines(
    vRGBA & picture, const Style & style, const TileData & tdataEx, const vRGBA & interpolatedPicEx)
{
    if(tdataEx.data.size() != 3)
        return;

    // convert data to knots - I use knots as standart units for vector rendering
    const UnitConverter converter = UnitConverter::make("knots", tdataEx.units.c_str());

    const vData & U_Ex = tdataEx.data[0];
    const vData & V_Ex = tdataEx.data[1];
    const vData & dataEx = tdataEx.data[2];

    constexpr int cell = 32; // must be 8, 16, 32, 64, 128

    for(size_t y = 0; y < TILE_SIZE; y += cell) {
        for(size_t x = 0; x < TILE_SIZE; x += cell) {
            const RGBA color = getVectorStyleColor(style, interpolatedPicEx, x, y);

            const float r = dataEx(x + 1, y + 1); // +1 Becouse of _Ex
            // Calc multiplier using converting to knots. 145 knots is maximum speed to render.
            const float muld = std::min(converter.backward(r), 145.0f) / (145.0f * cell);

            // forward
            {
                float fx = x;
                float fy = y;
                for(size_t i = cell * 4; --i;) {
                    if(0 <= fx && fx < TILE_SIZE && 0 <= fy && fy < TILE_SIZE) {
                        picture(size_t(fx), size_t(fy)) = color;
                        fx += interpolateF(U_Ex, fx + 1, fy + 1) * muld; // +1 Becouse of _Ex
                        fy -= interpolateF(V_Ex, fx + 1, fy + 1) * muld; // +1 Becouse of _Ex
                    } else
                        break;
                }
                // draw a head to show the direction
                if(2 < fx && fx < TILE_SIZE - 2 && 2 < fy && fy < TILE_SIZE - 2)
                    circle(picture, color, size_t(fx), size_t(fy), 1);
            }

            // backward
            {
                float bx = x;
                float by = y;
                for(size_t i = cell * 4; --i;) {
                    if(0 <= bx && bx < TILE_SIZE && 0 <= by && by < TILE_SIZE) {
                        picture(size_t(bx), size_t(by)) = color;
                        bx -= interpolateF(U_Ex, bx + 1, by + 1) * muld; // +1 Becouse of _Ex
                        by += interpolateF(V_Ex, bx + 1, by + 1) * muld; // +1 Becouse of _Ex
                    } else
                        break;
                }
            }

            // A "Hack" to include bottom and right borders
            if(x == 256 - cell)
                x = 256 - cell - 1;
        }
        if(y == 256 - cell)
            y = 256 - cell - 1;
    }
}
