#include "transport_router.h"

std::optional<OptimalRoute> Transport_router::GetOptimalRoute(std::string_view from, std::string_view to) const {

    std::optional<graph::Router<double>::RouteInfo> route_info
        = router_.BuildRoute(catalogue_.FindStop(from)->idx
                            ,catalogue_.FindStop(to)->idx);
    if (route_info == std::nullopt) {
        return std::nullopt;
    }
    OptimalRoute optimalRoute;
    for (const auto& edgeID : route_info.value().edges) {
        optimalRoute.items.push_back(id_of_Item_.at(edgeID));
    }
    optimalRoute.total_time = route_info.value().weight;
    return optimalRoute;
}

const Transport_router::Graph Transport_router::InitGraph() {
    Graph graph(catalogue_.GetStopsCount());
    for (const Route& bus : catalogue_.GetAllBuses()) {
        for (auto from_it = bus.stops.begin(); from_it!= bus.stops.end(); ++from_it) {
            double time = 0;
            int span_count = 0;
            const Stop* stop = catalogue_.FindStop(*from_it);
            for (auto to_it = std::next(from_it); to_it != bus.stops.end(); ++to_it) {
                time += (double(catalogue_.FindDistance(*std::prev(to_it), *to_it)) / routing_settings_.bus_velocity);
                graph::EdgeId id = graph.AddEdge({ stop->idx, catalogue_.FindStop(*to_it)->idx, time + routing_settings_.bus_wait_time });
                id_of_Item_.emplace(id, RouteItem{ stop , &bus , time , ++span_count });
            }
        }
    }
    return graph;
}