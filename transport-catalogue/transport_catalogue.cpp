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
void TransportCatalogue::AddDistance(constStopPtr first, constStopPtr second, const int& distance) {
    distances_[{first, second}] = distance;
}

TransportCatalogue::constRoutePtr TransportCatalogue::FindRoute(const string_view& name) const {
    return names_of_routes_.count(name) ? names_of_routes_.at(name) : nullptr;
}

TransportCatalogue::constStopPtr TransportCatalogue::FindStop(const string_view& name) const {
    return names_of_stops_.count(name) ? names_of_stops_.at(name) : nullptr;
}

int TransportCatalogue::FindDistance(const string_view& first, const string_view& second) const {
    pairConstStopPtr p(FindStop(first), FindStop(second));
    return distances_.count(p) ? distances_.at(p) : distances_.at({ p.second, p.first });
}

const TransportCatalogue::RouteStat TransportCatalogue::GetRoute(const string& name) const {
    constRoutePtr route = FindRoute(name);
    int route_distance = ComputeRouteDistance(route);
    return { route->stops.size(), unordered_set<string_view>(route->stops.begin(), route->stops.end()).size() ,route_distance,  route_distance/ComputeRouteLength(route)};
}

const set<string_view> TransportCatalogue::GetRoutesOfStop(const string& name_of_stop) const {
    return routes_of_stops_.at(name_of_stop);
}

double TransportCatalogue::ComputeRouteLength(constRoutePtr route) const {
    double route_length = 0;
    for (auto it = route->stops.begin(); it != route->stops.end()-1; it++) {
        route_length += ComputeDistance(FindStop(*it)->coordinates, FindStop(*next(it))->coordinates);
    }
    return route_length;
}

int TransportCatalogue::ComputeRouteDistance(constRoutePtr route) const {
    double route_distance = 0;
    for (auto it = route->stops.begin(); it != route->stops.end() - 1; it++) {
        route_distance += FindDistance(*it, *next(it));
    }
    return route_distance;
}