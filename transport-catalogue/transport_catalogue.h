#pragma once

#include <deque>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>

#include "geo.h"

namespace transport::core {
    
    class TransportCatalogue {
    public:

        struct Route {
            std::string name;
            std::vector<std::string> stops;
        };

        struct Stop {
            std::string name;
            geo::Coordinates coordinates;
        };
        struct RouteStat {
            size_t count_of_stops;
            size_t count_of_unique_stops;
            int route_length;
            double curvature;
        };

        using constRoutePtr = const Route*;
        using constStopPtr = const Stop*;
        using pairConstStopPtr = std::pair<constStopPtr, constStopPtr>;

        void AddRoute(const std::string& name, const std::vector<std::string_view>& stops);
        void AddStop(const std::string& name, const geo::Coordinates& coordinates);
        void AddDistance(const pairConstStopPtr& pair, const int& distance);

        constRoutePtr FindRoute(const std::string_view& name) const;
        constStopPtr FindStop(const std::string_view& name) const;
        int FindDistance(const std::string_view& first, const std::string_view& second) const;

        const RouteStat GetRoute(const std::string& name) const;
        const std::set<std::string_view> GetRoutesOfStop(const std::string& name_of_stop) const;
    private:
        struct Hasher {
            std::size_t operator()(const pairConstStopPtr& pair) const {
                return std::hash<constStopPtr>{}(pair.first) * 37 + std::hash<constStopPtr>{}(pair.second);
            }
        };

        std::deque<Route> routes_;
        std::deque<Stop> stops_;
        std::unordered_map<pairConstStopPtr, int, Hasher> distances_;

        std::unordered_map<std::string_view, constRoutePtr> names_of_routes_;
        std::unordered_map<std::string_view, constStopPtr> names_of_stops_;

        std::unordered_map<std::string_view, std::set<std::string_view>> routes_of_stops_;

        double ComputeRouteLength(constRoutePtr route) const;
        int ComputeRouteDistance(constRoutePtr route) const;
    };
}
    
