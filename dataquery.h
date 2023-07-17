#ifndef DATAQUERY_H
#define DATAQUERY_H

#include <string>
#include <vector>

struct TXYZoom
{
    size_t x, y, zoom;
};

struct DataQuery
{
#define D_file "FileName"
    std::string file;
#define D_style "StyleName"
    std::string style;
#define D_variables "Variables"
    std::vector<std::string> variables;
#define D_units "Units"
    std::string units;
#define D_time "Time"
    size_t time;

#define D_zoom "Zoom"
#define D_x "X"
#define D_y "Y"
    TXYZoom XYZoom;
    
    
};

#endif // DATAQUERY_H
