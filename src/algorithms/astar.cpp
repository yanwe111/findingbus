#include "algorithms/astar.h"
#include "utils/haversine.h"
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <chrono>

PathResult astar(
    const BusGraph& graph,
    const string& start_id,
    const string& end_id,
    WeightCriteria criteria
) {
    auto t_start = chrono::high_resolution_clock::now();
    PathResult result;

    auto start_it = graph.stops.find(start_id);
    auto end_it = graph.stops.find(end_id);
    if (start_it == graph.stops.end() || end_it == graph.stops.end()) {
        return result;
    }

    if (start_id == end_id) {
        result.path = {start_id};
        result.found = true;
        return result;
    }

    double goal_lat = end_it->second.lat;
    double goal_lng = end_it->second.lng;

    // Heuristic: straight-line distance to goal, scaled by criteria
    auto heuristic = [&](const string& stop_id) -> double {
        auto it = graph.stops.find(stop_id);
        if (it == graph.stops.end()) return 0.0;
        double h = haversine(it->second.lat, it->second.lng, goal_lat, goal_lng);
        switch (criteria) {
            case WeightCriteria::DISTANCE: return h;
            case WeightCriteria::TIME: return h / 60.0; // max speed ~60 km/h
            case WeightCriteria::PRICE: return 0.0; // no admissible heuristic for price
            case WeightCriteria::TRANSFERS: return 0.0;
        }
        return h;
    };

    // Min-heap: (f_score, stop_id) where f = g + h
    using PQEntry = pair<double, string>;
    priority_queue<PQEntry, vector<PQEntry>, greater<PQEntry>> pq;

    unordered_map<string, double> g_score;
    unordered_map<string, string> parent;
    unordered_map<string, string> parent_route;
    unordered_set<string> closed;

    g_score[start_id] = 0.0;
    pq.push({heuristic(start_id), start_id});

    while (!pq.empty()) {
        auto [f, u] = pq.top();
        pq.pop();

        if (closed.count(u)) continue;
        closed.insert(u);
        result.visited_nodes++;

        if (u == end_id) break;

        auto adj_it = graph.adjacency.find(u);
        if (adj_it == graph.adjacency.end()) continue;

        double g_u = g_score[u];

        for (auto& edge : adj_it->second) {
            if (closed.count(edge.to_stop_id)) continue;

            double w = calculate_weight(edge, criteria);
            double tentative_g = g_u + w;

            auto g_it = g_score.find(edge.to_stop_id);
            if (g_it == g_score.end() || tentative_g < g_it->second) {
                g_score[edge.to_stop_id] = tentative_g;
                parent[edge.to_stop_id] = u;
                parent_route[edge.to_stop_id] = edge.route_ref;
                double f_new = tentative_g + heuristic(edge.to_stop_id);
                pq.push({f_new, edge.to_stop_id});
            }
        }
    }

    // Reconstruct path
    if (closed.count(end_id)) {
        result.found = true;
        result.total_weight = g_score[end_id];

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
