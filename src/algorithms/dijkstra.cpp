#include "algorithms/dijkstra.h"
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <chrono>
#include <limits>

PathResult dijkstra(
    const BusGraph& graph,
    const string& start_id,
    const string& end_id,
    WeightCriteria criteria
) {
    auto t_start = chrono::high_resolution_clock::now();
    PathResult result;

    if (graph.stops.find(start_id) == graph.stops.end() ||
        graph.stops.find(end_id) == graph.stops.end()) {
        return result; // invalid stop IDs
    }

    if (start_id == end_id) {
        result.path = {start_id};
        result.found = true;
        return result;
    }

    // Min-heap: (distance, stop_id)
    using PQEntry = pair<double, string>;
    priority_queue<PQEntry, vector<PQEntry>, greater<PQEntry>> pq;

    unordered_map<string, double> dist;
    unordered_map<string, string> parent;      // stop -> previous stop
    unordered_map<string, string> parent_route; // stop -> route used to get here
    unordered_set<string> visited;

    dist[start_id] = 0.0;
    pq.push({0.0, start_id});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        if (visited.count(u)) continue;
        visited.insert(u);
        result.visited_nodes++;

        if (u == end_id) break;

        auto adj_it = graph.adjacency.find(u);
        if (adj_it == graph.adjacency.end()) continue;

        for (auto& edge : adj_it->second) {
            if (visited.count(edge.to_stop_id)) continue;

            double w = calculate_weight(edge, criteria);
            double new_dist = d + w;

            auto dist_it = dist.find(edge.to_stop_id);
            if (dist_it == dist.end() || new_dist < dist_it->second) {
                dist[edge.to_stop_id] = new_dist;
                parent[edge.to_stop_id] = u;
                parent_route[edge.to_stop_id] = edge.route_ref;
                pq.push({new_dist, edge.to_stop_id});
            }
        }
    }

    // Reconstruct path
    if (visited.count(end_id)) {
        result.found = true;
        result.total_weight = dist[end_id];

        // Build path in reverse (with cycle guard)
        string current = end_id;
        unordered_set<string> path_visited;
        while (current != start_id && path_visited.insert(current).second) {
            result.path.push_back(current);
            if (parent_route.count(current)) {
                result.routes_used.push_back(parent_route[current]);
            }
            current = parent[current];
        }
        result.path.push_back(start_id);
        reverse(result.path.begin(), result.path.end());
        reverse(result.routes_used.begin(), result.routes_used.end());

        // Count transfers (route changes)
        for (size_t i = 1; i < result.routes_used.size(); i++) {
            if (result.routes_used[i] != result.routes_used[i - 1]) {
                result.transfers++;
            }
        }
    }

    auto t_end = chrono::high_resolution_clock::now();
    result.execution_time_ms =
        chrono::duration<double, milli>(t_end - t_start).count();

    return result;
}
