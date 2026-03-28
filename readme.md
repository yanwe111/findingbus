# FindingBus - Vietnam Bus Route DSA Project 🚌

A Data Structures & Algorithms (DSA) educational project for modeling and finding optimal bus routes in Ho Chi Minh City and Dong Nai province, Vietnam.

## 🌟 Overview

FindingBus demonstrates practical DSA concepts through a real-world problem: efficiently routing buses across multiple provinces. The project uses graph-based algorithms to represent bus networks and find optimal paths based on real-world OpenStreetMap (OSM) data and manual datasets.

**Current Status:** Fully Functional (Production/Final).

- Integrated real OSM JSON data + manual route data.
- Implemented and benchmarked A\* and Dijkstra pathfinding algorithms.
- Supports fuzzy string matching for bus station queries.

## 🚀 Features

- **Real-World Graph Building:** Parses complex JSON data from OpenStreetMap and manual datasets to construct a directed graph of the bus network.
- **Smart Station Matching:** Utilizes Levenshtein edit distance algorithms to perform fuzzy string matching (e.g., finding the correct station even with minor typos or missing diacritics).
- **Advanced Pathfinding:** Implements both **Dijkstra** and **A\*** (A-Star) search algorithms to find the optimal path between stations.
- **Benchmarking Suite:** Includes an experimentation module to compare algorithmic performance (time execution and nodes visited).

## 📊 Performance (A\* vs Dijkstra)

Based on 100 random routing queries (Experiment 1):

- **Paths found:** 94/100
- **Dijkstra avg visited nodes:** 1993.6
- **A\* avg visited nodes:** 605.5 _(~69.6% node savings)_
- **Dijkstra avg time:** 19.5 ms
- **A\* avg time:** 6.9 ms _(~2.8x faster)_

## 🛠️ Tech Stack

- **Core Engine:** C++17
- **Data Fetching & Processing:** Python 3 (Overpass API)
- **Data Format:** JSON (via `nlohmann/json` for C++)

## 📁 Project Structure

```text
findingbus/
├── data/                      # Datasets & Output
│   ├── experiments/           # Benchmark results (.csv)
│   ├── graph/                 # Compiled graph data (stops.json, etc.)
│   ├── dong-nai-manual.json   # Manual coordinates & stations for Dong Nai
│   └── hcm-routes.json        # Processed OSM bus routes for HCMC
├── src/                       # C++ Source Code
│   ├── algorithms/            # Pathfinding algorithms (A*, Dijkstra)
│   ├── experiments/           # Benchmarking & testing modules
│   ├── graph/                 # Graph builders and unified loaders
│   ├── query/                 # CLI interface and fuzzy station matcher
│   ├── utils/                 # Utility functions
│   └── main.cpp               # Application entry point
├── scripts/                   # Python scripts for data fetching/parsing
└── readme.md                  # Project documentation
```
