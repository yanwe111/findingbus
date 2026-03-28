#include "graph/unified-loader.h"
#include "graph/stop-deduplicator.h"
#include "graph/graph-builder.h"
#include "query/cli-interface.h"
#include <iostream>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

int main(int argc, char* argv[]) {
    cout << "=== FindingBus - Bus Route Pathfinder ===\n\n";

    string osm_path = "data/hcm-routes.json";
    string manual_path = "data/dong-nai-manual.json";
    string graph_dir = "data/graph";

    if (argc > 1) osm_path = argv[1];
    if (argc > 2) manual_path = argv[2];

    if (!fs::exists(manual_path)) {
        cerr << "Error: Manual data not found at " << manual_path << "\n";
        return 1;
    }
    if (!fs::exists(osm_path)) {
        cerr << "Warning: OSM data not found at " << osm_path << "\n"
             << "Run: python3 scripts/osm-overpass-fetcher.py && "
             << "python3 scripts/osm-response-parser.py\n";
    }

    cout << "[1/4] Loading route data...\n";
    auto routes = load_all_routes(osm_path, manual_path);
    if (routes.empty()) {
        cerr << "Error: No routes loaded. Ensure data files exist.\n";
        return 1;
    }

    // Step 2: Deduplicate stops
    cout << "[2/4] Deduplicating stops...\n";
    auto stops = deduplicate_stops(routes, 0.05);

    // Step 3: Build graph
    cout << "[3/4] Building graph...\n";
    auto graph = build_graph(routes, stops, 2.0); // 2km transfer penalty

    // Step 4: Serialize graph
    cout << "[4/4] Serializing graph...\n";
    fs::create_directories(graph_dir);
    serialize_graph(graph, graph_dir);

    // Launch interactive CLI
    CliInterface cli;
    cli.run(graph);

    return 0;
}
