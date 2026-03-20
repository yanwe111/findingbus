#pragma once
#include <string>
#include <vector>
using namespace std;

struct Stop {
    string id;
    string name;
    double lat = 0.0;
    double lng = 0.0;
    vector<string> routes_served;
};