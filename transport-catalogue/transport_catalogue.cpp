#include <algorithm>
#include <stdexcept>
#include <unordered_set>

#include "transport_catalogue.h"

using namespace transport::core;
using namespace std;

void TransportCatalogue::AddRoute(const string& name, const vector<string_view>& stops) {
    routes_.emplace_back(move(Route{ name, {stops.begin(),stops.end()} }));
    names_of_routes_[routes_.back().name] = &routes_.back();
    for (auto& it : names_of_routes_[routes_.back().name]->stops) {
        routes_of_stops_[it].emplace(routes_.back().name);
    }
}

void TransportCatalogue::AddStop(const string& name, const geo::Coordinates& coordinates) {
    stops_.emplace_back(move(Stop{ name, coordinates }));
    names_of_stops_[stops_.back().name] = &stops_.back();
    routes_of_stops_[stops_.back().name];
}

TransportCatalogue::constRoutePtr TransportCatalogue::FindRoute(const string& name) const {
    return names_of_routes_.count(name) ? names_of_routes_.at(name) : nullptr;
}

TransportCatalogue::constStopPtr TransportCatalogue::FindStop(const string& name) const {
    return names_of_stops_.count(name) ? names_of_stops_.at(name) : nullptr;
}

const TransportCatalogue::RouteStat TransportCatalogue::GetRoute(const string& name) const {
    const Route* route = FindRoute(name);
    return { route->stops.size(), unordered_set<string_view>(route->stops.begin(), route->stops.end()).size() , ComputeRouteLength(route)};
}

const std::set<std::string_view> TransportCatalogue::GetRoutesOfStop(const string& name_of_stop) const {
    return routes_of_stops_.at(name_of_stop);
}

double TransportCatalogue::ComputeRouteLength(constRoutePtr route) const {
    double route_length = 0;
    for (auto it = route->stops.begin(); it != route->stops.end()-1; it++) {
        route_length += ComputeDistance(FindStop(*it)->coordinates, FindStop(*next(it))->coordinates);
    }
    return route_length;
}