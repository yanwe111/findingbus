#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "graph/stop.h"
#include "graph/unified-loader.h"

using namespace std;

/// Stops within DEDUP_RADIUS_KM are merged into one canonical stop.
/// Returns map of stop_id -> Stop with routes_served populated.
unordered_map<string, Stop> deduplicate_stops(
    const vector<LoadedRoute>& routes,
    double dedup_radius_km = 0.05 // 50 meters
);
