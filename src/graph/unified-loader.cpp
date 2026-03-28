#include "graph/unified-loader.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <unordered_set>

using json = nlohmann::json;

/// Parse OSM JSON into LoadedRoute objects
static vector<LoadedRoute> load_osm_routes(const string& path) {
    vector<LoadedRoute> routes;
    ifstream file(path);
    if (!file.is_open()) {
        cerr << "[unified-loader] Cannot open OSM file: " << path << "\n";
        return routes;
    }
    try {
        json data = json::parse(file);
        for (auto& r : data["routes"]) {
            LoadedRoute lr;
            lr.ref = r.value("ref", "");
            lr.name = r.value("name", "");
            lr.from = r.value("from", "");
            lr.to = r.value("to", "");
            lr.network = r.value("network", "");
            lr.source = "osm";

            // Parse charge field: "15000 VND" or "15.000" etc.
            string charge = r.value("charge", "");
            if (!charge.empty()) {
                string digits;
                for (char c : charge) {
                    if (isdigit(c)) digits += c;
                }
                if (!digits.empty()) lr.price_vnd = stoi(digits);
            }

            // Parse stops
            if (r.contains("stops")) {
                for (auto& s : r["stops"]) {
                    Stop stop;
                    stop.name = s.value("name", "Unnamed Stop");
                    stop.lat = s.value("lat", 0.0);
                    stop.lng = s.value("lng", 0.0);
                    if (stop.lat != 0.0 && stop.lng != 0.0) {
                        lr.stops.push_back(stop);
                    }
                }
            }
            routes.push_back(move(lr));
        }
    } catch (const exception& e) {
        cerr << "[unified-loader] Error parsing OSM JSON: " << e.what() << "\n";
    }
    return routes;
}

/// Parse manual Dong Nai JSON into LoadedRoute objects
static vector<LoadedRoute> load_manual_routes(const string& path) {
    vector<LoadedRoute> routes;
    ifstream file(path);
    if (!file.is_open()) {
        cerr << "[unified-loader] Cannot open manual file: " << path
                  << " (non-fatal, continuing with OSM data only)\n";
        return routes;
    }

    try {
        json data = json::parse(file);
        // Handle both array format and {"routes": [...]} format
        json& route_array = data.is_array() ? data : data["routes"];
        for (auto& r : route_array) {
            LoadedRoute lr;
            lr.ref = r.value("number", "");
            lr.name = r.value("name", "");
            lr.direction = r.value("direction", "");
            lr.price_vnd = r.value("price", 0);
            lr.source = "manual";
            lr.network = "Dong Nai Manual";

            if (r.contains("waypoints")) {
                for (auto& w : r["waypoints"]) {
                    Stop stop;
                    stop.lat = w.value("lat", 0.0);
                    stop.lng = w.value("lng", 0.0);
                    stop.name = w.value("name", "Waypoint");
                    if (stop.lat != 0.0 && stop.lng != 0.0) {
                        lr.stops.push_back(stop);
                    }
                }
            }
            routes.push_back(move(lr));
        }
    } catch (const exception& e) {
        cerr << "[unified-loader] Error parsing manual JSON: " << e.what() << "\n";
    }
    return routes;
}

vector<LoadedRoute> load_all_routes(
    const string& osm_json_path,
    const string& manual_json_path
) {
    auto osm_routes = load_osm_routes(osm_json_path);
    auto manual_routes = load_manual_routes(manual_json_path);

    cout << "[unified-loader] Loaded " << osm_routes.size() << " OSM routes, "
              << manual_routes.size() << " manual routes\n";

    // OSM data is primary; add manual routes that don't overlap
    // A manual route overlaps if an OSM route has the same ref
    unordered_set<string> osm_refs;
    for (auto& r : osm_routes) osm_refs.insert(r.ref);

    int added_manual = 0;
    for (auto& mr : manual_routes) {
        if (osm_refs.find(mr.ref) == osm_refs.end()) {
            osm_routes.push_back(move(mr));
            added_manual++;
        }
    }
    cout << "[unified-loader] Added " << added_manual
              << " non-overlapping manual routes\n";

    return osm_routes;
}
