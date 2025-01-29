#pragma once
#include <string>
#include <vector>
#include "geo.h"
struct Route {
    std::string name;
    std::vector<std::string> stops;
    std::string last_stop;
};
struct Stop {
    std::string name;
    geo::Coordinates coordinates;
    size_t idx = 0;
};
