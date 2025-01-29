#pragma once

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

using TrCatalogue = transport::core::TransportCatalogue;

json::Document LoadJSON(std::istream& input);

void ParseJson(const json::Document& document, TrCatalogue& catalogue);