#pragma once

#include <deque>
#include <unordered_map>
#include <set>

#include "domain.h"
#include "geo.h"

namespace transport::core {
    
    class TransportCatalogue {
    public:
        
        struct RouteStat {
            size_t count_of_stops = 0;
            size_t count_of_unique_stops = 0;
            int route_length = 0;
            double curvature = 0.0;
        };

        using constRoutePtr = const Route*;
        using constStopPtr = const Stop*;
 
        void AddRoute(const std::string& name, const std::vector<std::string_view>& stops, const std::string& last_stop);
        void AddStop(const std::string& name, const geo::Coordinates& coordinates);
        void AddDistance(constStopPtr first, constStopPtr second, const int& distance);

        std::deque<Route> GetSortedRoutes() const;
        std::deque<Stop> GetSortedStops() const;
        
        constRoutePtr FindRoute(const std::string_view name) const;
        constStopPtr FindStop(const std::string_view name) const;
        int FindDistance(const std::string_view first, const std::string_view second) const;

        const RouteStat GetRoute(const std::string_view name) const;
        const std::set<std::string_view> GetRoutesOfStop(const std::string_view name_of_stop) const;
    private:
        using pairConstStopPtr = std::pair<constStopPtr, constStopPtr>;

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
    
