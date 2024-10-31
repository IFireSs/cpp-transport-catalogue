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
            double route_length;
        };

        using constRoutePtr = const Route*;
        using constStopPtr = const Stop*;

        void AddRoute(const std::string& name, const std::vector<std::string_view>& stops);
        void AddStop(const std::string& name, const geo::Coordinates& coordinates);

        constRoutePtr FindRoute(const std::string& name) const;
        constStopPtr FindStop(const std::string& name) const;

        const RouteStat GetRoute(const std::string& name) const;
        const std::set<std::string_view> GetRoutesOfStop(const std::string& name_of_stop) const;
    private:
        std::deque<Route> routes_;
        std::deque<Stop> stops_;

        std::unordered_map<std::string_view, constRoutePtr> names_of_routes_;
        std::unordered_map<std::string_view, constStopPtr> names_of_stops_;

        std::unordered_map<std::string_view, std::set<std::string_view>> routes_of_stops_;
        
        double ComputeRouteLength(constRoutePtr route) const;
    };
}
    
