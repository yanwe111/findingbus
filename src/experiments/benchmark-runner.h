#pragma once

#include "graph/graph-builder.h"
#include <string>

using namespace std;

// Chay thi nghiem benchmark va luu ket qua and save results to output_dir
void run_benchmarks(const BusGraph& graph, const string& output_dir);
