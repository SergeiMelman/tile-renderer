#ifndef CLIENTDATA_H
#define CLIENTDATA_H

#include "types.h"
#include "json_helpers.h"
#include "datafile.h"
#include "unitconverter.h"

#include <shared_mutex>

// Тут будут храниться
// - Общие стили для тайлов одной сессии
// - конвертированные CLUT
// - Файл(ы) данных открытые.
// - Посчитанные для них lon-lat
// - Загруженные слои.

class ClientDataCache
{
public:
    std::string styles_str;
    StylesMap stylesMap;
    FilesMap filesMap;
    VariablesMap variablesMap;
    UnitConvertersMap convertersMap;

public:
    void set(const std::string & str, json & j);

    const Style & getStyle(const DataQuery & dataQ) const;
    const DataFile & getFile(const DataQuery & dataQ);
    const VariableData & getVariableData(const DataQuery & dataQ);
    const UnitConverter & getUnitConverter(const Style & style, const VariableData & variableData);
    
};

using pClientDataCache = ClientDataCache *;

#endif // CLIENTDATA_H
