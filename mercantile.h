#ifndef MERCANTILE_H
#define MERCANTILE_H

#include <cmath>

/// Stolen from:
/// http://www.maptiler.org/google-maps-coordinates-tile-bounds-projection/
/// recoded from Py to Cpp by Sergei
/// "tileSize" could be removed, but I'm lazy

#define TILE_SIZE 256

class Mercantile
{
private:
    static constexpr double tileSize = TILE_SIZE; // = 256
    static constexpr double initialResolution
        = 2.0 * double(M_PI) * 6378137.0 / tileSize; // = 2 * math.pi * 6378137 / self.tileSize //#
                                                     // 156543.03392804062 for tileSize 256 pixels
    static constexpr double originShift = 2.0 * double(M_PI) * 6378137.0
        / 2.0; // = 2.0 * math.pi * 6378137 / 2.0 //# 20037508.342789244
public:
    struct Coord
    {
        double lat, lon;
    };

    struct Box
    {
        Coord min, max;
    };

public:
    static double Resolution(const double zoom)
    {
        //"Resolution (meters/pixel) for given zoom level (measured at Equator)"
        // return (2 * math.pi * 6378137) / (self.tileSize * 2**zoom)
        return initialResolution / std::pow(2.0, zoom);
    }
    static Coord PixelsToMeters(const double px, const double py, const double zoom)
    {
        //"Converts pixel coordinates in given zoom level of pyramid to EPSG:900913"
        const double res = Resolution(zoom);
        return {px * res - originShift, py * res - originShift};
    }
    static Coord MetersToLatLon(const Coord & c)
    {
        return MetersToLatLon(c.lat, c.lon);
    }
    static Coord MetersToLatLon(const double mx, const double my)
    {
        //"Converts XY point from Spherical Mercator EPSG:900913 to lat/lon in WGS84 Datum"
        double lon = (mx / originShift) * 180.0;
        double lat = (my / originShift) * 180.0;
        lat = 180.0 / double(M_PI)
            * (2.0 * atan(exp(lat * double(M_PI) / 180.0)) - double(M_PI) / 2.0);

        // +360 is because a calculation of longitude is from -180 to 180, but we need [0, 360]
        return {lat, lon < 0 ? lon + 360 : lon};
    }
    static Coord PicPixelsToLatLon(
        const double x, const double y, const double tx, const double ty, const double zoom)
    {
        const Coord m = PixelsToMeters(tx * tileSize + x, ty * tileSize + y, zoom);
        Coord coord = MetersToLatLon(m);
        return coord;
    }
    static Box TileBoundsInMeters(const double tx, const double ty, const double zoom)
    {
        //"Returns bounds of the given tile in EPSG:900913 coordinates"
        auto min = PixelsToMeters(tx * tileSize, ty * tileSize, zoom);
        auto max = PixelsToMeters((tx + 1) * tileSize, (ty + 1) * tileSize, zoom);
        return {min, max};
    }
    static Box TileLatLonBounds(const int tx, const int ty, const int zoom)
    {
        //"Returns bounds of the given tile in latutude/longitude using WGS84 datum"
        const Box bounds = TileBoundsInMeters(tx, ty, zoom);
        Coord min = MetersToLatLon(bounds.min.lat, bounds.min.lon);
        Coord max = MetersToLatLon(bounds.max.lat, bounds.max.lon);
        return {min, max};
    }
};

#endif // MERCANTILE_H
