#include "experiments/benchmark-runner.h"
#include "algorithms/dijkstra.h"
#include "algorithms/astar.h"
#include <fstream>
#include <iostream>
#include <random>
#include <vector>
#include <iomanip>

/// Collect all stop IDs from graph into a vector for random sampling
static vector<string> collect_stop_ids(const BusGraph& graph) {
    vector<string> ids;
    ids.reserve(graph.stops.size());
    for (auto& [id, _] : graph.stops) ids.push_back(id);
    return ids;
}

void run_benchmarks(const BusGraph& graph, const string& output_dir) {
    auto stop_ids = collect_stop_ids(graph);
    if (stop_ids.size() < 2) {
        cout << "[benchmark] Not enough stops for benchmarking.\n";
        return;
    }

    mt19937 rng(42); // fixed seed for reproducibility
    uniform_int_distribution<size_t> dist(0, stop_ids.size() - 1);

    // Generate 100 random (start, end) pairs
    constexpr int NUM_QUERIES = 100;
    vector<pair<string, string>> queries;
    for (int i = 0; i < NUM_QUERIES; i++) {
        string s = stop_ids[dist(rng)];
        string e = stop_ids[dist(rng)];
        while (s == e) e = stop_ids[dist(rng)];
        queries.push_back({s, e});
    }

    // Experiment 1: Dijkstra vs A* performance comparison
    cout << "\n=== Experiment 1: Dijkstra vs A* (100 queries) ===\n";
    ofstream csv(output_dir + "/benchmark-results.csv");
    csv << "query,dijkstra_visited,dijkstra_ms,astar_visited,astar_ms,"
        << "dijkstra_weight,astar_weight,same_result\n";

    int total_found = 0;
    double dijkstra_total_visited = 0, astar_total_visited = 0;
    double dijkstra_total_ms = 0, astar_total_ms = 0;

    for (int i = 0; i < NUM_QUERIES; i++) {
        auto& [s, e] = queries[i];
        auto d_result = dijkstra(graph, s, e, WeightCriteria::DISTANCE);
        auto a_result = astar(graph, s, e, WeightCriteria::DISTANCE);

        bool same = (abs(d_result.total_weight - a_result.total_weight) < 0.001);
        csv << i << "," << d_result.visited_nodes << "," << d_result.execution_time_ms
            << "," << a_result.visited_nodes << "," << a_result.execution_time_ms
            << "," << d_result.total_weight << "," << a_result.total_weight
            << "," << (same ? "yes" : "NO") << "\n";

        if (d_result.found) {
            total_found++;
            dijkstra_total_visited += d_result.visited_nodes;
            astar_total_visited += a_result.visited_nodes;
            dijkstra_total_ms += d_result.execution_time_ms;
            astar_total_ms += a_result.execution_time_ms;
        }
    }

    double savings = (total_found > 0)
        ? (1.0 - astar_total_visited / dijkstra_total_visited) * 100.0
        : 0.0;
    cout << fixed << setprecision(1);
    cout << "  Paths found: " << total_found << "/" << NUM_QUERIES << "\n";
    cout << "  Dijkstra avg visited: " << dijkstra_total_visited / max(1, total_found) << "\n";
    cout << "  A* avg visited: " << astar_total_visited / max(1, total_found) << "\n";
    cout << "  A* node savings: " << savings << "%\n";
    cout << "  Dijkstra avg time: " << dijkstra_total_ms / max(1, total_found) << " ms\n";
    cout << "  A* avg time: " << astar_total_ms / max(1, total_found) << " ms\n";

    csv.close();
    cout << "\nResults saved to " << output_dir << "/benchmark-results.csv\n";
}
