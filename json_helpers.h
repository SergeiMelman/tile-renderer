#ifndef JSON_HELPERS_H
#define JSON_HELPERS_H

#include "nlohmann/json.hpp"

struct Style;
struct DataQuery;

using nlohmann::json;

void from_json(const json & j, Style & style);
void from_json(const json & j, DataQuery & dataQ);

const json & fillStylesJson(json & j_styles);

#endif // JSON_HELPERS_H
