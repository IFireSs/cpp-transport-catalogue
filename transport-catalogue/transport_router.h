#pragma once

#include "router.h"
#include "transport_catalogue.h"

struct RoutingSettings {
    int bus_wait_time;
    double bus_velocity;
};
struct RouteItem {
    const Stop* stop;
    const Route* bus;
    double time = 0.0;
    int span_count = 0;
};
struct OptimalRoute {
    double total_time = 0.0;
    std::vector<RouteItem> items;
};

class Transport_router {
public:
    Transport_router(const transport::core::TransportCatalogue& catalogue, const RoutingSettings& routing_settings)
        : catalogue_(catalogue)
        , routing_settings_(routing_settings)
        , graph_(InitGraph())
        , router_(graph_)
    {
    }
    std::optional<OptimalRoute> GetOptimalRoute(std::string_view from, std::string_view to) const;
private:
    using Graph = graph::DirectedWeightedGraph<double>;
    const transport::core::TransportCatalogue& catalogue_;
    const RoutingSettings& routing_settings_; 
    std::unordered_map<graph::EdgeId, RouteItem> id_of_Item_;
    const Graph graph_;
    const Graph InitGraph();
    const graph::Router<double> router_;
};


