#include "json_reader.h"
#include "json_builder.h"
#include <iomanip>

using namespace std::literals;
using namespace json;

Document LoadJSON(std::istream& input) {
    return Load(input);
}

static void ParseStops(const Array& arr, TrCatalogue& catalogue) {
    for (const auto& request : arr) {
        const auto& dict = request.AsMap();
        if (dict.find("type"s)->second.AsString() == "Stop"s) {
            catalogue.AddStop(dict.at("name"s).AsString()
                , { dict.at("latitude"s).AsDouble()
                , dict.at("longitude"s).AsDouble() });
        }
    }
}
static void ParseDistances(const Array& arr, TrCatalogue& catalogue) {
    for (const auto& request : arr) {
        const auto& dict = request.AsMap();
        if (dict.find("type"s)->second.AsString() == "Stop"s) {
            TrCatalogue::constStopPtr first_stop = catalogue.FindStop(dict.at("name"s).AsString());
            for (const auto& [second_stop, distance] : dict.at("road_distances"s).AsMap()) {
                catalogue.AddDistance(first_stop, catalogue.FindStop(second_stop), distance.AsInt());
            }
        }
    }
}
static void ParseRoutes(const Array& arr, TrCatalogue& catalogue) {
    for (const auto& request : arr) {
        const auto& dict = request.AsMap();
        if (dict.find("type"s)->second.AsString() == "Bus"s) {
            std::vector<std::string_view> stops;
            for (const auto& stop : dict.at("stops"s).AsArray()) {
                stops.push_back(stop.AsString());
            }
            if (!dict.at("is_roundtrip"s).AsBool()) {
                std::string last_stop(stops.back());
                stops.insert(stops.end(), std::next(stops.rbegin()), stops.rend());
                catalogue.AddRoute(dict.at("name"s).AsString(), stops, last_stop);
            }
            else {
                catalogue.AddRoute(dict.at("name"s).AsString(), stops, std::string(stops.back()));
            }
            
        }
    }
}
json::Node RequestError(int id) {
    return json::Builder()
        .StartDict()
        .Key("request_id"s).Value(id)
        .Key("error_message"s).Value("not found"s)
        .EndDict()
        .Build();
}

static void PrintStat(const Array& arr, TrCatalogue& catalogue, const RenderSettings& rs) {
    Array result;
    for (const auto& request : arr) {
        const auto& dict = request.AsMap();
        const auto& request_type = dict.find("type"s);
        
        if (request_type->second.AsString() == "Bus"s) {
            auto& name = dict.at("name").AsString();
            if (catalogue.FindRoute(name)) {
                const auto [count_of_stops, count_of_unique_stops, route_length, curvature] = catalogue.GetRoute(name);
                result.emplace_back(json::Builder()
                    .StartDict()
                    .Key("curvature"s).Value(curvature)
                    .Key("request_id"s).Value(dict.at("id").AsInt())
                    .Key("route_length"s).Value(route_length)
                    .Key("stop_count"s).Value(static_cast<int>(count_of_stops))
                    .Key("unique_stop_count"s).Value(static_cast<int>(count_of_unique_stops))
                    .EndDict()
                    .Build());
            }
            else {
                result.emplace_back(RequestError(dict.at("id").AsInt()));
            }
        }
        else if (request_type->second.AsString() == "Stop"s) {
            auto& name = dict.at("name").AsString();
            if (catalogue.FindStop(name)) {
                std::set<std::string_view> routes = catalogue.GetRoutesOfStop(name);
                Array arr;
                for (auto& route : routes) {
                    arr.emplace_back(std::string(route));
                }
                result.emplace_back(json::Builder()
                            .StartDict()
                            .Key("buses"s).Value(arr)
                            .Key("request_id"s).Value(dict.at("id").AsInt())
                            .EndDict()
                            .Build());
            }
            else {
                result.emplace_back(RequestError(dict.at("id").AsInt()));
            }
        }
        else if (request_type->second.AsString() == "Map"s) {
            MapRenderer rndr(rs, catalogue.GetSortedRoutes(), catalogue.GetSortedStops());
            result.emplace_back(json::Builder()
                .StartDict()
                .Key("map"s).Value(rndr.GetMap())
                .Key("request_id"s).Value(dict.at("id").AsInt())
                .EndDict()
                .Build());
        }
    }
    Print(Document{ result }, std::cout);
}
static svg::Color ParseColor(const Node& node) {
    if (node.IsString()) {
        return node.AsString();
    }
    auto& arr = node.AsArray();
    if (arr.size() == 3) {
        return svg::Rgb{ static_cast<uint8_t>(arr[0].AsInt()), static_cast<uint8_t>(arr[1].AsInt()),
                        static_cast<uint8_t>(arr[2].AsInt()) };
    }
    else {
        return svg::Rgba{ static_cast<uint8_t>(arr[0].AsInt()), static_cast<uint8_t>(arr[1].AsInt()),
                     static_cast<uint8_t>(arr[2].AsInt()), arr[3].AsDouble() };
    }
}
static RenderSettings ParseRenderSettings(const Dict& dict) {
    RenderSettings render_settings;
    render_settings.width = dict.at("width").AsDouble(); 
    render_settings.height = dict.at("height").AsDouble(); 
    render_settings.padding = dict.at("padding").AsDouble(); 
    render_settings.line_width = dict.at("line_width").AsDouble(); 
    render_settings.stop_radius = dict.at("stop_radius").AsDouble();
    render_settings.bus_label_font_size = dict.at("bus_label_font_size").AsInt();
    auto& offset = dict.at("bus_label_offset"s).AsArray();
    render_settings.bus_label_offset.dx = offset[0].AsDouble();
    render_settings.bus_label_offset.dy = offset[1].AsDouble();
    render_settings.stop_label_font_size = dict.at("stop_label_font_size").AsDouble();
    auto& offset_ = dict.at("stop_label_offset"s).AsArray();
    render_settings.stop_label_offset.dx = offset_[0].AsDouble();
    render_settings.stop_label_offset.dy = offset_[1].AsDouble();
    render_settings.underlayer_color = ParseColor(dict.at("underlayer_color"));
    render_settings.underlayer_width = dict.at("underlayer_width"s).AsDouble();
    for (const auto& it : dict.at("color_palette"s).AsArray()) {
        render_settings.color_palette.push_back(ParseColor(it));
    }
    return render_settings;
}

void ParseJson(const Document& document, TrCatalogue& catalogue) { 
    const Dict& root  = document.GetRoot().AsMap();
    if (const auto& base_requests = root.find("base_requests"s); base_requests != root.end()) {
        const Array& arr = base_requests->second.AsArray();
        ParseStops(arr, catalogue);
        ParseDistances(arr, catalogue);
        ParseRoutes(arr, catalogue);
    }
    RenderSettings rs;
    if (const auto& render_settings = root.find("render_settings"s); render_settings != root.end()) {
        rs = ParseRenderSettings(render_settings->second.AsMap());
    }
    if (const auto& stat_requests = root.find("stat_requests"s); stat_requests != root.end()) {
        const Array& arr = stat_requests->second.AsArray();
        if (!arr.empty()) {
            PrintStat(arr, catalogue, rs);
        }
    }
}
