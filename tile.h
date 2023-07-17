#ifndef TILE_H
#define TILE_H

#include "types.h"


struct TXYZoom;
    
class TileCoords
{
    struct Pixel1DCoord
    {
        float ind_t;

        size_t getIndex() const {return size_t(ind_t);};
        float getT() const {return ind_t - size_t(ind_t);};
    };
    using vPixel1DCoord = std::vector<Pixel1DCoord>;
    
    vPixel1DCoord lon; // X
    vPixel1DCoord lat; // Y
    
public:
    static TileCoords make(const LonLat & lonlat, const TXYZoom &XYZoom);
    
    TileData interpolate(const VariableData & variable) const;
};

#endif // TILE_H
