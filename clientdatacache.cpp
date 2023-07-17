#include "clientdatacache.h"
#include "dataquery.h"
#include "grody/webserver.h"
#include "style.h"
#include "tools.h"
#include "datafile.h"

#include <iostream>
#include <map>

int
on_connection_created(struct client * client)
{
    std::cout << "on_connection_created" << std::endl;
    client->data = new ClientDataCache;
    return 0;
}

void
on_connection_destruction(struct client * client)
{
    std::cout << "on_connection_destruction" << std::endl;
    delete pClientDataCache(client->data);
}

void
ClientDataCache::set(const std::string & str, nlohmann::json & j)
{
    /// NOTE: Data race condition should be resolved for multithreading

    const size_t & spos = str.find("Styles");
    const auto & styles_str_ = str.substr(spos);

    if(styles_str != styles_str_) {
        styles_str = styles_str_;
        fillStylesJson(j["Styles"]).get_to(stylesMap);
    }
    
    if(stylesMap.empty()) {
        stylesMap["default"] = Style{};
    }
}

const Style &
ClientDataCache::getStyle(const DataQuery & dataQ) const
{
    return stylesMap.at(getStyleNameFromMap(stylesMap, dataQ));
}

const DataFile &
ClientDataCache::getFile(const DataQuery & dataQ)
{
    // if there is no dataQ.file it will be created in files and loaded.
    // otherwise load does nothing, just return DataFile
    return filesMap[dataQ.file].load(dataQ.file);
}

const VariableData &
ClientDataCache::getVariableData(const DataQuery & dataQ)
{
    return filesMap[dataQ.file].getVariableData(dataQ.variables, dataQ.time);
}

const UnitConverter &
ClientDataCache::getUnitConverter(const Style & style, const VariableData & variableData)
{
    auto & converter = convertersMap[{style.units, variableData.units}];

    if(!converter)
        converter = UnitConverter::make(style.units.c_str(), variableData.units.c_str());

    return converter;
}
