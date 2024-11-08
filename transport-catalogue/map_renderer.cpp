#include "map_renderer.h"

#include <sstream>


std::deque<geo::Coordinates> MapRenderer::FillCoordinstes(const std::deque<Stop>& stops_) {
    std::deque<geo::Coordinates> coordinates;
    for (auto& stop : stops_) {
        if (0 == std::count_if(routes_.begin(), routes_.end(), [&stop](const Route& route) {
            return std::find(route.stops.begin(), route.stops.end(), stop.name) != route.stops.end(); })) {
            continue;
        }
        coordinates.push_back(stop.coordinates);
    }
    return coordinates;
}

const geo::Coordinates& MapRenderer::FindCoordinatesOfName(const std::string_view name){
    return (*std::find_if(stops_.begin(), stops_.end(), [&](const Stop& stop) {return stop.name == name;})).coordinates;
}

const std::string& MapRenderer::FindStopOfCoordinates(const geo::Coordinates& coordinates){
    return (*std::find_if(stops_.begin(), stops_.end(), [&](const Stop& stop) {return stop.coordinates == coordinates; })).name;
}

const svg::Polyline MapRenderer::RenderPoliline(const Route& route, const int& id) {

    std::deque<svg::Point> stops_points;
    for (std::string_view stop : route.stops) {
        stops_points.push_back(sphereProjector_(FindCoordinatesOfName(stop)));
    }
    svg::Polyline polyline;
    polyline.SetFillColor(svg::NoneColor).SetStrokeColor(render_settings_.color_palette[id])
        .SetStrokeWidth(render_settings_.line_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    for (const auto& stop_point : stops_points) {
        polyline.AddPoint(stop_point);
    }
    return polyline;
}
const svg::Text MapRenderer::RenderRouteNameUnderLayer(const std::string& text, const svg::Point& point) {
    svg::Text route_name_underlayer;
    route_name_underlayer.SetData(text).SetPosition(point).SetFillColor(render_settings_.underlayer_color)
        .SetStrokeColor(render_settings_.underlayer_color)
        .SetStrokeWidth(render_settings_.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetOffset({ render_settings_.bus_label_offset.dx, render_settings_.bus_label_offset.dy })
        .SetFontSize(static_cast<uint32_t>(render_settings_.bus_label_font_size))
        .SetFontFamily("Verdana").SetFontWeight("bold");
    return route_name_underlayer;
}
const svg::Text MapRenderer::RenderRouteName(const std::string& text, const svg::Point& point, const int& id) {
    svg::Text route_name;
    route_name.SetData(text).SetPosition(point).SetFillColor(render_settings_.color_palette[id])
        .SetOffset({ render_settings_.bus_label_offset.dx, render_settings_.bus_label_offset.dy })
        .SetFontSize(static_cast<uint32_t>(render_settings_.bus_label_font_size))
        .SetFontFamily("Verdana").SetFontWeight("bold");
    return route_name;
}
const svg::Circle MapRenderer::RenderStopPoint(const geo::Coordinates& point) {
    svg::Circle circle;
    circle.SetCenter(sphereProjector_(point)).SetRadius(render_settings_.stop_radius).SetFillColor("white"s);
    return circle;
}
const svg::Text MapRenderer::RenderStopNameUnderLayer(const geo::Coordinates& point) {
    svg::Text underlayer;
    underlayer.SetData(FindStopOfCoordinates(point)).SetPosition(sphereProjector_(point))
        .SetFillColor(render_settings_.underlayer_color).SetStrokeColor(render_settings_.underlayer_color)
        .SetStrokeWidth(render_settings_.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetOffset({ render_settings_.stop_label_offset.dx, render_settings_.stop_label_offset.dy })
        .SetFontSize(static_cast<uint32_t>(render_settings_.stop_label_font_size)).SetFontFamily("Verdana");
    return underlayer;
}
const svg::Text MapRenderer::RenderStopName(const geo::Coordinates& point) {
    svg::Text route_name;
    route_name.SetData(FindStopOfCoordinates(point)).SetPosition(sphereProjector_(point)).SetFillColor("black"s)
        .SetOffset({ render_settings_.stop_label_offset.dx, render_settings_.stop_label_offset.dy })
        .SetFontSize(static_cast<uint32_t>(render_settings_.stop_label_font_size)).SetFontFamily("Verdana");
    return route_name;
}

void MapRenderer::RenderMap() {
    svg::Document result;
    size_t id = 0;
    for (const auto& route : routes_) {
        result.Add(RenderPoliline(route, id));
        id + 1 < render_settings_.color_palette.size() ? id++ : id = 0;
    }
    id = 0;
    for (const auto& route : routes_) {
        result.Add(RenderRouteNameUnderLayer(route.name, sphereProjector_(FindCoordinatesOfName(route.stops.front()))));
        result.Add(RenderRouteName(route.name, sphereProjector_(FindCoordinatesOfName(route.stops.front())), id));
        if (route.stops.front()!=route.last_stop) {
            result.Add(RenderRouteNameUnderLayer(route.name, sphereProjector_(FindCoordinatesOfName(route.last_stop))));
            result.Add(RenderRouteName(route.name, sphereProjector_(FindCoordinatesOfName(route.last_stop)), id));
        }
        id + 1 < render_settings_.color_palette.size() ? id++ : id = 0;
    }
    for (const auto& point : coordinates_) {
        result.Add(RenderStopPoint(point));
    }
    for (const auto& point : coordinates_) {
        result.Add(RenderStopNameUnderLayer(point));
        result.Add(RenderStopName(point));
    }
    std::ostringstream result_str;
    result.Render(result_str);
    map_ = result_str.str();
}

const std::string MapRenderer::GetMap() const{
    return map_;
}