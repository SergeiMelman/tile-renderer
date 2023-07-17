#include "tile.h"
#include "mercantile.h"
#include "types.h"
#include "dataquery.h"
#include <algorithm>

/// \brief findIndex - finds such index in 'v' that: v[index] < d < v[index + 1]
/// Usefull for interpolation purpose.
/// \param v - array
/// \param d - value
/// \return 0 <= index < (v.size() - 2)
size_t
findIndex(const vDouble & v, double d)
{
    // +1 чтобы можно было -1(*) сделать. -1 чтобы не выходило за пределы при интерполяции
    // -1(*) сдвиг на левый индекс от найденной позиции
    auto it_lon = std::upper_bound(v.begin() + 1, v.end() - 1, d) - 1;
    return size_t(it_lon - v.begin());
}

///
/// \brief Tile::make - creates conformity for each pixel in picture to
/// lon, lat indexes with interpolation parameter
/// it ueses TILE_SIZE + 2 for extrapolation on borders.
/// \param lonlat - two arrays - 2D grid (could be irregular)
/// \return
///
TileCoords
TileCoords::make(const LonLat & lonlat, const TXYZoom & XYZoom)
{
    // tested
    TileCoords tileCoords;
    tileCoords.lon.resize(TILE_SIZE + 2);
    tileCoords.lat.resize(TILE_SIZE + 2);

    auto coord = Mercantile::PicPixelsToLatLon(0, 0, XYZoom.x, XYZoom.y, XYZoom.zoom);
    // first index should be found this way
    auto & vlon = lonlat.lon;
    size_t coord_ind_lon = findIndex(vlon, coord.lon);

    auto & vlat = lonlat.lat;
    size_t coord_ind_lat = findIndex(vlat, coord.lat);

    // note: pixels are in the 1D array
    auto & lon = tileCoords.lon;
    auto lonIter = lon.begin();
    // 'rbegin' for the vertical data flip. '+2' becouse the last 2 are for the getTileDataEx
    auto & lat = tileCoords.lat;
    auto latIter = lat.rbegin() + 2;
    for(size_t pix_ind = 0; pix_ind < TILE_SIZE;) {
        // calc "t" for pixel
        const auto t_lon = float(
            (coord.lon - vlon[coord_ind_lon]) / (vlon[coord_ind_lon + 1] - vlon[coord_ind_lon]));
        *lonIter++ = {coord_ind_lon + t_lon};

        const auto t_lat = float(
            (coord.lat - vlat[coord_ind_lat]) / (vlat[coord_ind_lat + 1] - vlat[coord_ind_lat]));
        *latIter++ = {coord_ind_lat + t_lat};

        ++pix_ind;
        coord = Mercantile::PicPixelsToLatLon(pix_ind, pix_ind, XYZoom.x, XYZoom.y, XYZoom.zoom);

        // для очередного pix_ind, зная его pix_in_coord найдем
        // ближайший левый индекс coord_ind из набора.
        while(coord_ind_lon + 2 < vlon.size() // +2 потому что при расчете t стоит +1
            && vlon[coord_ind_lon + 1] < coord.lon)
            ++coord_ind_lon;

        while(coord_ind_lat + 2 < vlat.size() // +2 потому что при расчете t стоит +1
            && vlat[coord_ind_lat + 1] < coord.lat)
            ++coord_ind_lat;
    }

    /// Idea: for pictures with extended (1 + 256 + 1)^2 boundaries
    /// for right-boundary pixel coord[256] position is used - as naturally.
    /// for the left boundary coord[257] position is used. This is to support both
    /// ncField::getTileData() and ncField::getTileDataEx() with no code modification.

    // calculations for the left border
    {
        const float a0 = lon[0].ind_t;
        const float a1 = lon[1].ind_t;
        const float new_a = std::max(a0 - (a1 - a0), 0.0f); // shift beyond the left boundary :)
        // and put it to the last position
        *(lon.end() - 1) = {new_a};
    }

    // calculations for the right border
    {
        const float a0 = lon[TILE_SIZE - 2].ind_t;
        const float a1 = lon[TILE_SIZE - 1].ind_t;
        // shift beyond the right boundary :)
        const float new_a = std::min(a1 + (a1 - a0), float(vlon.size() - 1));
        // and put it to the prev-last position
        *(lon.end() - 2) = {new_a};
    }

    // calculations for the upper border
    {
        const float a0 = lat[0].ind_t;
        const float a1 = lat[1].ind_t;
        float new_a = a0 - (a1 - a0); // shift beyond the upper boundary :)
        if(new_a < 0)
            new_a = 0;
        // and put it to the last position
        *(lat.end() - 1) = {new_a};
    }

    // calculations for the bottom border
    {
        const float a0 = lat[TILE_SIZE - 2].ind_t;
        const float a1 = lat[TILE_SIZE - 1].ind_t;
        float new_a = a1 + (a1 - a0); // shift beyond the bootom boundary :)
        if(new_a > vlat.size() - 1)
            new_a = vlat.size() - 1;
        // and put it to the prev-last position
        *(lat.end() - 2) = {new_a};
    }

    return tileCoords;
}

TileData
TileCoords::interpolate(const VariableData & variable) const
{
    TileData tileEx;
    tileEx.units = variable.units;
    tileEx.data.reserve(variable.data.size());
    for(auto varData : variable.data) {
        vData & tileDataEx = tileEx.data.emplace_back(TILE_SIZE + 2, TILE_SIZE + 2);
        auto tileDataExIter = tileDataEx.begin();

        for(size_t y = 0; y < TILE_SIZE + 2; ++y) {
            // this formula is because the lon lat was calculated for extended borders
            // that is why for pixel y === 1 we should use lat[0], etc
            const auto py = lat[y != 0 ? y - 1 : TILE_SIZE + 1];
            const size_t pyindex = py.getIndex();
            const float pyt = py.getT();
            for(size_t x = 0; x < TILE_SIZE + 2; ++x) {
                // the formula for x is similar to y
                const auto px = lon[x != 0 ? x - 1 : TILE_SIZE + 1];

                const size_t pxindex = px.getIndex();
                const float up_left = varData(pxindex, pyindex);
                const float up_right = varData(pxindex + 1, pyindex);
                const float bot_left = varData(pxindex, pyindex + 1);
                const float bot_right = varData(pxindex + 1, pyindex + 1);

                const float pxt = px.getT();
                const float ul_ur = up_left + (up_right - up_left) * pxt;
                const float bl_br = bot_left + (bot_right - bot_left) * pxt;

                const float u_b = ul_ur + (bl_br - ul_ur) * pyt;

                *tileDataExIter++ = u_b;
            }
        }
    }

    return tileEx;
}
