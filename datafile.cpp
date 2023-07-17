#include "datafile.h"
#include <cmath>
#include <algorithm>


vData
DataFile::loadVariable(const std::string & variable_name, size_t time) const
{
    vData data(count[2], count[1]);
    data_file.getVar(variable_name).getVar({time, 0, 0}, count, data.data());
    return data;
}

std::string
DataFile::loadVariableUnits(const std::string & variable_name) const
{
    std::string units;
    data_file.getVar(variable_name).getAtt("units").getValues(units);
    return units;
}

DataFile &
DataFile::load(const std::string & file_name_)
{
    if(file_name_ != file_name) {
        file_name.clear();
        try {
            data_file.open(file_name_, netCDF::NcFile::read);
            lonlat.lat.resize(data_file.getDim("latitude").getSize());
            lonlat.lon.resize(data_file.getDim("longitude").getSize());
            // vtime.resize(data_file.getDim("time").getSize());
            data_file.getVar("longitude").getVar(lonlat.lon.data());
            data_file.getVar("latitude").getVar(lonlat.lat.data());
            // data_file.getVar("time").getVar(vtime.data());
        } catch(const netCDF::exceptions::NcException & ex) {
            throw std::invalid_argument("NCDF File read error: " + file_name_ + ":" + ex.what());
        }
        file_name = file_name_;
        count = {1, lonlat.lat.size(), lonlat.lon.size()};
    }
    return *this;
}


inline float
sqr(float d)
{
    return d * d;
}

const VariableData &
DataFile::getVariableData(const vString & variables, size_t time)
{
    // cache works only for 'time'
    if(variablesTime == time) {
        return variableDatasMap[variables];
    }

    variableDatasMap.clear();
    VariableData & variable = variableDatasMap[variables];

    const size_t sz = variables.size();
    variable.data.reserve(sz + 1);
    variable.units = loadVariableUnits(variables.front()); // must be identical for all variables!

    for(auto & var : variables) {
        variable.data.push_back(loadVariable(var, time));
    }
    
    // if variable is a vectorfield then calc velocities
    if(sz == 2) {
        vData & D = variable.data.emplace_back(count[2], count[1]);
        auto pU = variable.data[0].cbegin();
        auto pV = variable.data[1].cbegin();
        for(float & d : D)
            d = std::sqrt(sqr(*pU++) + sqr(*pV++));
//        auto mm = std::minmax_element(D.begin(), D.end());
    }

    variablesTime = time;

    return variable;
}

const LonLat &
DataFile::getLonLat() const
{
    return lonlat;
}
