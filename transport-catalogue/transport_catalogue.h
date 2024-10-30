#pragma once

#include <deque>
#include <string>
#include <tuple>
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

        void AddRoute(const std::string& name, const std::vector<std::string_view>& stops);
        void AddStop(const std::string& name, const geo::Coordinates& coordinates);

        const Route* FindRoute(const std::string& name) const;
        const Stop* FindStop(const std::string& name) const;

        const std::tuple<size_t, size_t, double> GetRoute(const std::string& name) const;
        const std::set<std::string_view> GetStop(const std::string& name) const;
    private:
        std::deque<Route> routes_;
        std::deque<Stop> stops_;

        std::unordered_map<std::string_view, const Route*> names_of_routes_;
        std::unordered_map<std::string_view, const Stop*> names_of_stops_;

        std::unordered_map<std::string_view, std::set<std::string_view>> routes_of_stops_;
        
        double ComputeRouteLength(const Route* route) const;
    };
}
    
