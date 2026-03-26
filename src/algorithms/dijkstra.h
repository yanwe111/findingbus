#pragma once

#include "algorithms/path-result.h"
#include "graph/graph-builder.h"
#include "graph/weight-criteria.h"

// Thuat toan Dijkstra tim duong ngan nhat
/// @param graph The bus network graph
/// @param start_id Source stop ID
/// @param end_id Destination stop ID
/// @param criteria Weight criterion for optimization
/// @return PathResult with path, weight, routes, and benchmarking stats
PathResult dijkstra(
    const BusGraph& graph,
    const string& start_id,
    const string& end_id,
    WeightCriteria criteria = WeightCriteria::DISTANCE
);
