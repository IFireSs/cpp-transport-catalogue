#pragma once

#include "geo.h"
#include "svg.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <deque>
#include "domain.h"

inline const double EPSILON = 1e-6;

class SphereProjector {
public:
    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_ = 0.0;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};
struct Offset{
    double dx = 0.0;
    double dy = 0.0;
};

struct RenderSettings {
    double width = 0;
    double height = 0;
    double padding = 0;
    double line_width = 0;
    double stop_radius = 1.0;
    int bus_label_font_size = 1;
    Offset bus_label_offset;
    int stop_label_font_size = 1;
    Offset stop_label_offset;
    svg::Color underlayer_color;
    double underlayer_width = 0.0;
    std::deque<svg::Color> color_palette;
};


class MapRenderer {
public:
    MapRenderer(const RenderSettings& render_settings, std::deque<Route> routes, std::deque<Stop> stops) :
        render_settings_(render_settings), routes_(std::move(routes)), stops_(std::move(stops))
        , coordinates_(std::move(FillCoordinstes(stops_)))
        , sphereProjector_(SphereProjector(coordinates_.begin(), coordinates_.end(), render_settings_.width, render_settings_.height, render_settings_.padding)) {
        RenderMap();
    }

    const std::string GetMap() const ;
private:
    const RenderSettings& render_settings_;
    const std::deque<Route> routes_;
    const std::deque<Stop> stops_;
    const std::deque<geo::Coordinates> coordinates_;
    const SphereProjector sphereProjector_;
    std::string map_;

    void RenderMap();

    std::deque<geo::Coordinates> FillCoordinstes(const std::deque<Stop>& stops_);

    const geo::Coordinates& FindCoordinatesOfName(const std::string_view& name);
    const std::string& FindStopOfCoordinates(const geo::Coordinates& coordinates);

    const svg::Polyline RenderPoliline(const Route& route, const int& id);
    const svg::Text RenderRouteNameUnderLayer(const std::string& text, const svg::Point& point);
    const svg::Text RenderRouteName(const std::string& text, const svg::Point& point, const int& id);
    const svg::Circle RenderStopPoint(const geo::Coordinates& point);
    const svg::Text RenderStopNameUnderLayer(const geo::Coordinates& point);
    const svg::Text RenderStopName(const geo::Coordinates& point);
};



