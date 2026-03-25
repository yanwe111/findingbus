#pragma once

#include <string>
#include <vector>

using namespace std;

struct PathResult {
    vector<string> path;         // ordered stop IDs from start to end
    double total_weight = 0.0;             // total weight (interpretation depends on criteria)
    vector<string> routes_used;  // route refs used along the path
    int transfers = 0;                     // number of route changes
    int visited_nodes = 0;                 // nodes explored (for benchmarking)
    double execution_time_ms = 0.0;        // wall-clock time in milliseconds
    bool found = false;                    // true if a valid path was found
};
