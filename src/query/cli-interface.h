#pragma once

#include "graph/graph-builder.h"
#include "algorithms/path-result.h"
#include "query/station-matcher.h"

// Giao dien dong lenh tuong tac de truy van tuyen xe buyt
class CliInterface {
public:
    void run(const BusGraph& graph);

private:
    StationMatcher matcher_;

    void cmd_find(const BusGraph& graph, const string& args);
    void cmd_info(const BusGraph& graph, const string& args);
    void cmd_stats(const BusGraph& graph);
    void cmd_benchmark(const BusGraph& graph);
    void cmd_help();
    void print_path_result(const BusGraph& graph, const PathResult& r, int index = -1);
};
