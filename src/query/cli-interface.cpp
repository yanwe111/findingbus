#include "query/cli-interface.h"
#include "algorithms/dijkstra.h"
#include "algorithms/astar.h"
#include "experiments/benchmark-runner.h"
#include <iostream>
#include <sstream>
#include <iomanip>

void CliInterface::print_path_result(const BusGraph& graph, const PathResult& r, int index) {
    if (!r.found) {
        cout << "  No path found.\n";
        return;
    }
    if (index >= 0) cout << "\n--- Path " << (index + 1) << " ---\n";
    else cout << "\n--- Shortest Path ---\n";

    cout << "  Stops: " << r.path.size()
              << " | Weight: " << fixed << setprecision(2) << r.total_weight
              << " | Transfers: " << r.transfers
              << " | Visited: " << r.visited_nodes
              << " | Time: " << r.execution_time_ms << " ms\n";

    // Show path with stop names
    cout << "  Route: ";
    string prev_route;
    for (size_t i = 0; i < r.path.size(); i++) {
        auto it = graph.stops.find(r.path[i]);
        string name = (it != graph.stops.end()) ? it->second.name : r.path[i];

        if (i < r.routes_used.size() && r.routes_used[i] != prev_route) {
            if (!prev_route.empty()) cout << " [transfer] ";
            cout << "[Route " << r.routes_used[i] << "] ";
            prev_route = r.routes_used[i];
        }
        if (i > 0) cout << " -> ";
        cout << name;
    }
    cout << "\n";
}

void CliInterface::cmd_find(const BusGraph& graph, const string& args) {
    // Parse: "from_name" "to_name" [--by distance|time|price|transfers]
    // Simple parsing: split by " to " or "--"
    string from_str, to_str;
    WeightCriteria criteria = WeightCriteria::DISTANCE;

    // Check for --by flag
    string cleaned = args;
    auto by_pos = cleaned.find("--by ");
    if (by_pos != string::npos) {
        string criterion = cleaned.substr(by_pos + 5);
        cleaned = cleaned.substr(0, by_pos);
        // Trim
        while (!cleaned.empty() && cleaned.back() == ' ') cleaned.pop_back();

        if (criterion.find("time") != string::npos) criteria = WeightCriteria::TIME;
        else if (criterion.find("price") != string::npos) criteria = WeightCriteria::PRICE;
        else if (criterion.find("transfer") != string::npos) criteria = WeightCriteria::TRANSFERS;
    }

    // Split by " to "
    auto to_pos = cleaned.find(" to ");
    if (to_pos == string::npos) {
        cout << "Usage: find <from> to <to> [--by distance|time|price|transfers]\n";
        return;
    }
    from_str = cleaned.substr(0, to_pos);
    to_str = cleaned.substr(to_pos + 4);

    // Fuzzy match stations
    auto from_matches = matcher_.match(from_str, 1);
    auto to_matches = matcher_.match(to_str, 1);

    if (from_matches.empty()) { cout << "No station found for: " << from_str << "\n"; return; }
    if (to_matches.empty()) { cout << "No station found for: " << to_str << "\n"; return; }

    string from_id = from_matches[0].stop_id;
    string to_id = to_matches[0].stop_id;
    cout << "From: " << from_matches[0].name << " (" << from_id << ")\n";
    cout << "To: " << to_matches[0].name << " (" << to_id << ")\n";
    cout << "Criteria: " << criteria_to_string(criteria) << "\n";

    // Run A* (primary)
    auto result = astar(graph, from_id, to_id, criteria);
    print_path_result(graph, result);
}

void CliInterface::cmd_info(const BusGraph& graph, const string& args) {
    string route_ref = args;
    // Trim whitespace
    while (!route_ref.empty() && route_ref.front() == ' ') route_ref.erase(0, 1);
    while (!route_ref.empty() && route_ref.back() == ' ') route_ref.pop_back();

    // Find stops served by this route
    int stop_count = 0;
    cout << "\n--- Route " << route_ref << " ---\n";
    for (auto& [id, s] : graph.stops) {
        for (auto& r : s.routes_served) {
            if (r == route_ref) { stop_count++; break; }
        }
    }
    cout << "  Stops: " << stop_count << "\n";
}

void CliInterface::cmd_benchmark(const BusGraph& graph) {
    run_benchmarks(graph, "data/experiments");
}

void CliInterface::cmd_help() {
    cout << "\n=== FindingBus CLI ===\n"
              << "Commands:\n"
              << "  find <from> to <to> [--by distance|time|price|transfers]\n"
              << "  info <route_ref>\n"
              << "  stats\n"
              << "  benchmark\n"
              << "  help\n"
              << "  quit\n\n";
}

void CliInterface::run(const BusGraph& graph) {
    matcher_.load_stops(graph.stops);
    cmd_help();

    string line;
    while (true) {
        cout << "findingbus> ";
        if (!getline(cin, line)) break;

        // Trim
        while (!line.empty() && line.front() == ' ') line.erase(0, 1);
        while (!line.empty() && line.back() == ' ') line.pop_back();
        if (line.empty()) continue;

        if (line == "quit" || line == "exit") break;
        else if (line == "help") cmd_help();
        else if (line == "benchmark") cmd_benchmark(graph);
        else if (line.substr(0, 4) == "find") cmd_find(graph, line.substr(4));
        else if (line.substr(0, 4) == "info") cmd_info(graph, line.substr(4));
        else cout << "Unknown command. Type 'help' for usage.\n";
    }
    cout << "Goodbye!\n";
}
