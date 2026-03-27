#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "stop.h"
#include "graph/unified-loader.h"

using namespace std;

// The core data structure stores the entire bus network as a mathematical graph (including a list of stops and connecting routes).
struct BusGraph {
    unordered_map<string, Stop> stops; // list of stops 
    unordered_map<string, vector<Edge>> adjacency; // list of connecting routes

    int node_count() const { return static_cast<int>(stops.size()); } // stops counting 
    
    // connecting routes counting 
    int edge_count() const {
        int total = 0;
        for (auto& [_, edges] : adjacency) total += static_cast<int>(edges.size());
        return total;
    }
};

// Declare the graph construction function.
BusGraph build_graph(
    const vector<LoadedRoute>& routes,
    const unordered_map<string, Stop>& canonical_stops,
    double transfer_penalty = 2.0
);

/// Declare a function to find the nearest station.
string find_nearest_stop_id(
    const unordered_map<string, Stop>& stops,
    double lat, double lng,
    double max_radius = 0.1
);

/// Serialize graph to JSON files
void serialize_graph(const BusGraph& graph, const string& output_dir);
