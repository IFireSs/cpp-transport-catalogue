#include "stat_reader.h"

#include <iomanip>
#include <set>

void transport::stat::ParseAndPrint(const core::TransportCatalogue& transport_catalogue, std::string_view request,
    std::ostream& output) {
    size_t space = request.find(' ');
    auto request_type = request.substr(0, space);
    std::string name = std::string(request.substr(space + 1, request.find_last_not_of(' ') - space));
    if (request_type == "Bus") {
        if (transport_catalogue.FindRoute(name)) {
            const auto [count_of_stops, count_of_unique_stops, route_length,  curvature] = transport_catalogue.GetRoute(name);
            output << "Bus " << name << ": " << count_of_stops << " stops on route, " << count_of_unique_stops << " unique stops, " << std::setprecision(6) << route_length << " route length, " << std::setprecision(6) << curvature << " curvature"<< std::endl;
            return;
        }
        output << "Bus " << name << ": not found" << std::endl;
    }
    else if (request_type == "Stop") {
        if (transport_catalogue.FindStop(name)) {
            std::set<std::string_view> routes = transport_catalogue.GetRoutesOfStop(name);
            if (!routes.empty()) {
                output << "Stop " << name << ": buses";
                for (auto& route : routes) {
                    output << " " << route;
                }
                output << std::endl;
                return;
            }
            output << "Stop " << name << ": no buses" << std::endl;
            return;
        }
        output << "Stop " << name << ": not found" << std::endl;
    }
}