#pragma once

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

using TrCatalogue = transport::core::TransportCatalogue;

json::Document LoadJSON(std::istream& input);

void ParseJson(const json::Document& document, TrCatalogue& catalogue);
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */