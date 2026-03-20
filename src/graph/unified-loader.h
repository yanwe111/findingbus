#pragma once

#include <string>
#include <vector>
#include "graph/stop.h"

using namespace std;

// Route bus loaded from OSM
struct LoadedRoute {
    string ref;
    string name;
    string from;
    string to;
    int price_vnd = 0;
    string network;
    string direction;
    vector<Stop> stops;
    string source; // "osm" or "manual"
};

/// Load all routes from both OSM and manual JSON data files
vector<LoadedRoute> load_all_routes(
    const string& osm_json_path,
    const string& manual_json_path
);
