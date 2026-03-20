#include "graph/graph-builder.h"
#include "utils/haversine.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <set>

using json = nlohmann::json;

// Find the nearest station with given lat and long coordinates.
string find_nearest_stop_id(
    const unordered_map<string, Stop>& stops,
    double lat, double lng,
    double max_radius
) {
    string best_id;
    double best_distance = max_radius + 1.0;
    for (auto& [id, s] : stops) {
        double curr_distance = haversine(lat, lng, s.lat, s.lng); // Calculate the distance from the origin to the station under consideration.
        if (curr_distance < best_distance) {
            best_distance = curr_distance;
            best_id = id;
        }
    }
    return best_id; // return nearest stop ID
}

// Transform the list of bus routes into a directed graph.
BusGraph build_graph(
    const vector<LoadedRoute>& routes, // List of raw bus routes.
    const unordered_map<string, Stop>& canonical_stops, // List of standardized stations.
    double transfer_penalty // Penalty for changing routes
) {
    BusGraph graph;
    graph.stops = canonical_stops; // Use the normalized list of stations as nodes (vertices) for the graph.

    // Connecting consecutive stations on the same line together to form a chain.
    for (auto& route : routes) {

        // Match the coordinates of each stop and consolidate them into a single variable (stop_ids) available on the map.
        vector<string> stop_ids;
        for (auto& s : route.stops) {
            string id = find_nearest_stop_id(canonical_stops, s.lat, s.lng);
            if (!id.empty()) stop_ids.push_back(id);
        }
        // Add edges between consecutive stops i and i+1
        for (size_t i = 0; i + 1 < stop_ids.size(); i++) {
            auto& from_stop = canonical_stops.at(stop_ids[i]);
            auto& to_stop = canonical_stops.at(stop_ids[i + 1]);

            Edge e;
            e.to_stop_id = stop_ids[i + 1];
            e.distance_km = haversine(from_stop.lat, from_stop.lng, to_stop.lat, to_stop.lng);
            e.route_ref = route.ref;
            e.price_vnd = route.price_vnd;
            e.direction = route.direction;
            e.is_transfer = false; // Mark this as a straight route, NOT a transfer.

            graph.adjacency[stop_ids[i]].push_back(e);
        }
    }

    // Develop a system for vehicle switching between routes
    int transfer_edges = 0; // The variable counts the number of transition edges created.
    for (auto& [stop_id, stop] : graph.stops) {
        if (stop.routes_served.size() < 2) continue;  

        auto it = graph.adjacency.find(stop_id); // Find a list of directions from this station.
        if (it == graph.adjacency.end()) continue;

        // Group the directions by route.
        unordered_map<string, vector<Edge>> by_route;
        for (auto& e : it->second) {
            if (!e.is_transfer) by_route[e.route_ref].push_back(e);
        }
        if (by_route.size() < 2) continue;

        // For each pair of routes, add transfer penalty edges from this stop to next stops on the OTHER route
        for (auto& [r1, _] : by_route) {
            for (auto& [r2, edges_r2] : by_route) {
                if (r1 == r2) continue;
                for (auto& e : edges_r2) {
                    Edge te; //  a virtual edge to represent getting off the bus and switching to a new route.
                    te.to_stop_id = e.to_stop_id;
                    te.distance_km = e.distance_km + transfer_penalty;
                    te.route_ref = r2;
                    te.is_transfer = true;
                    te.price_vnd = e.price_vnd;
                    graph.adjacency[stop_id].push_back(te);
                    transfer_edges++;
                }
            }
        }
    }

    //Return to the complete graph
    cout << "Built graph: " << graph.node_count() << " nodes, " << graph.edge_count() << " edges (" << transfer_edges << " transfer)\n";
    return graph;
}



void serialize_graph(const BusGraph& graph, const string& output_dir) {
    // Serialize stops index
    json stops_json = json::array();
    for (auto& [id, s] : graph.stops) {
        stops_json.push_back({
            {"id", s.id}, {"name", s.name},
            {"lat", s.lat}, {"lng", s.lng},
            {"routes_served", s.routes_served}
        });
    }
    ofstream stops_file(output_dir + "/stops.json");
    if (stops_file.is_open()) {
        stops_file << stops_json.dump(2);
        cout << "[graph-builder] Saved " << stops_json.size()
                  << " stops to " << output_dir << "/stops.json\n";
    }

    // Serialize adjacency list
    json adj_json = json::object();
    for (auto& [stop_id, edges] : graph.adjacency) {
        json edge_list = json::array();
        for (auto& e : edges) {
            edge_list.push_back({
                {"to", e.to_stop_id}, {"dist_km", e.distance_km},
                {"route", e.route_ref}, {"price", e.price_vnd},
                {"transfer", e.is_transfer}
            });
        }
        adj_json[stop_id] = edge_list;
    }
    ofstream adj_file(output_dir + "/adjacency-list.json");
    if (adj_file.is_open()) {
        adj_file << adj_json.dump(2);
        cout << "[graph-builder] Saved adjacency list to "
                  << output_dir << "/adjacency-list.json\n";
    }
}
