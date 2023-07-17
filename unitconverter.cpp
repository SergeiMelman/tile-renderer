#include "unitconverter.h"
#include "tools.h"

#include <iostream>
#include <cstring>

static const std::shared_ptr<ut_system> unitSystem(ut_read_xml(nullptr), ut_free_system);

UnitConverter
UnitConverter::make(const char * const u1, const char * const u2)
{
    if(std::strcmp(u1, u2) == 0)
        return {}; // nothing to convert

    if(!unitSystem)
        throw std::runtime_error(STR("Unable to find UDUNITS xml files."));

    UnitConverter converter;

    converter.unit1.reset(ut_parse(unitSystem.get(), u1, UT_ASCII), ut_free);
    if(!converter.unit1)
        throw std::invalid_argument(STR("Unable to parse <unit1>: " + u1));

    converter.unit2.reset(ut_parse(unitSystem.get(), u2, UT_ASCII), ut_free);
    if(!converter.unit2)
        throw std::invalid_argument(STR("Unable to parse <unit2>: " + u2));

    const int convertible = ut_are_convertible(converter.unit1.get(), converter.unit2.get());
    if(!convertible)
        throw std::invalid_argument(STR("Units: " + u1 + " and " + u2 + " - are not convertable!"));

    converter.converterF.reset(
        ut_get_converter(converter.unit1.get(), converter.unit2.get()), cv_free);
    converter.converterB.reset(
        ut_get_converter(converter.unit2.get(), converter.unit1.get()), cv_free);

    return converter;
}

float UnitConverter::forward(float c) const
{
    return converterF ? cv_convert_float(converterF.get(), c) : c;
}

float UnitConverter::backward(float c) const
{
    return converterB ? cv_convert_float(converterB.get(), c) : c;
}

/*
// All temperaure convertions are going thru Kelvin
// All speed convertions are going thru m/s
float
convertMy(const std::string & from, const std::string & to, const float C)
{
    const static std::map<std::string, float (*)(const float)> mapFrom = {
        {"c", [](const float C) -> float { return C + 273.15f; }},
        {"k", [](const float C) -> float { return C; }},
        {"f", [](const float C) -> float { return ((C - 32.0f) * 5.0f / 9.0f) + 273.15f; }},
        {"m/s", [](const float C) -> float { return C; }},
        {"kmh", [](const float C) -> float { return C / 3.6f; }},
        {"knot", [](const float C) -> float { return C / 1.944f; }},
        {"mh", [](const float C) -> float { return C / 2.237f; }}, // miles/hour
        {"fs", [](const float C) -> float { return C / 3.281f; }}, // foots/sec
    };

    const static std::map<std::string, float (*)(const float)> mapTo = {
        {"c", [](const float C) -> float { return C - 273.15f; }},
        {"k", [](const float C) -> float { return C; }},
        {"f", [](const float C) -> float { return (C - 273.15f) * 9.0f / 5.0f + 32.0f; }},
        {"m/s", [](const float C) -> float { return C; }},
        {"kmh", [](const float C) -> float { return C * 3.6f; }},
        {"knot", [](const float C) -> float { return C * 1.944f; }},
        {"mh", [](const float C) -> float { return C * 2.237f; }}, // miles/hour
        {"fs", [](const float C) -> float { return C * 3.281f; }}, // foots/sec

    };

    try {
        return mapTo.at(to)(mapFrom.at(from)(C));
    } catch(...) {
        return C;
    }
}

*/
