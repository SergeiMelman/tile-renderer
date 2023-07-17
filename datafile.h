#ifndef DATAFILE_H
#define DATAFILE_H

#include <netcdf>

#include "types.h"

class DataFile
{
private:
    netCDF::NcFile data_file;
    std::string file_name; // load
    LonLat lonlat; // load
    //vDouble vtime;  // not needed??
    
    std::vector<size_t> count; // for getVariableData
    std::map<vString, VariableData> variableDatasMap;
    size_t variablesTime{static_cast<size_t>(-1)};
    
private:
    vData loadVariable(const std::string & variable_name, size_t time) const;
    std::string loadVariableUnits(const std::string & variable_name) const;
public:
    //DataFile();
    DataFile & load(const std::string & file_name_);
    const VariableData & getVariableData(const vString & variables, size_t time);
    const LonLat & getLonLat() const;
};

#endif // DATAFILE_H
