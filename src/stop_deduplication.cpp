#include "graph/stop-deduplicator.h"
#include "utils/haversine.h"
#include <algorithm>
#include <iostream>
#include <sstream>

/// Generate a stable stop ID from coordinates (rounded to ~10m precision)
static string make_stop_id(double lat, double lng) {
    ostringstream oss;
    oss << fixed;
    oss.precision(4);
    oss << "stop_" << lat << "_" << lng;
    return oss.str();
}

unordered_map<string, Stop> deduplicate_stops(
    const vector<LoadedRoute>& routes,
    double dedup_radius_km
) {
    // Collect all raw stops with their route refs
    struct RawStop {
        string name;
        double lat, lng;
        string route_ref;
    };
    vector<RawStop> all_stops;

    for (auto& route : routes) {
        for (auto& s : route.stops) {
            all_stops.push_back({s.name, s.lat, s.lng, route.ref});
        }
    }

    // Greedy clustering: assign each stop to nearest existing cluster or create new
    unordered_map<string, Stop> canonical;
    vector<string> cluster_ids; // ordered list for consistent nearest neighbor search

    for (auto& raw : all_stops) {
        string best_id;
        double best_dist = dedup_radius_km + 1.0;

        // Find nearest existing canonical stop within radius
        for (auto& cid : cluster_ids) {
            auto& cs = canonical[cid];
            double dist = haversine(raw.lat, raw.lng, cs.lat, cs.lng);
            if (dist < best_dist) {
                best_dist = dist;
                best_id = cid;
            }
        }

        if (best_dist <= dedup_radius_km && !best_id.empty()) {
            // Merge into existing cluster
            auto& cs = canonical[best_id];
            // Prefer named stop over "Waypoint" or "Unnamed Stop"
            if ((cs.name == "Waypoint" || cs.name == "Unnamed Stop") &&
                raw.name != "Waypoint" && raw.name != "Unnamed Stop") {
                cs.name = raw.name;
            }
            // Add route ref if not already present
            if (find(cs.routes_served.begin(), cs.routes_served.end(),
                          raw.route_ref) == cs.routes_served.end()) {
                cs.routes_served.push_back(raw.route_ref);
            }
        } else {
            // Create new canonical stop
            string id = make_stop_id(raw.lat, raw.lng);
            // Handle ID collision by appending suffix
            if (canonical.count(id)) {
                int suffix = 2;
                while (canonical.count(id + "_" + to_string(suffix))) suffix++;
                id = id + "_" + to_string(suffix);
            }
            Stop s;
            s.id = id;
            s.name = raw.name;
            s.lat = raw.lat;
            s.lng = raw.lng;
            s.routes_served.push_back(raw.route_ref);
            canonical[id] = s;
            cluster_ids.push_back(id);
        }
    }
    return canonical;
}
