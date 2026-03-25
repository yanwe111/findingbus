#pragma once

#include "algorithms/path-result.h"
#include "graph/graph-builder.h"
#include "graph/weight-criteria.h"

// A* tim duong ngan nhat voi heuristic Haversine
// Duyet it node hon Dijkstra nho ham uoc luong khoang cach duong chim bay
PathResult astar(
    const BusGraph& graph,
    const string& start_id,
    const string& end_id,
    WeightCriteria criteria = WeightCriteria::DISTANCE
);
