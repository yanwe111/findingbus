#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "graph/weight-criteria.h"
#include "graph/stop.h"
using namespace std;

struct MatchResult {
    string stop_id;
    string name;
    double score; // lower = better match
};

/// Fuzzy station name matcher.
/// Normalizes Vietnamese diacritics and does substring + edit distance matching.
class StationMatcher {
public:
    void load_stops(const unordered_map<string, Stop>& stops);

    /// Find top-N matching stops for a query string
    vector<MatchResult> match(const string& query, int top_n = 3) const;

private:
    struct StopEntry {
        string id;
        string name;
        string normalized; // lowercase, stripped diacritics
    };
    vector<StopEntry> entries_;

    static string normalize(const string& s);
    static int edit_distance(const string& a, const string& b);
};
