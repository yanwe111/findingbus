#pragma once

#include <string>
#include <vector>

using namespace std;

// Tieu chi trong so cho bai toan tim duong ngan nhat
enum class WeightCriteria {
    DISTANCE,   // Haversine km
    TIME,       // distance / avg_speed (25 km/h)
    PRICE,      // route price in VND
    TRANSFERS   // 1.0 per transfer, 0.0 for same-route
};

/// A directed edge between two stops
struct Edge {
    string to_stop_id;
    double distance_km = 0.0;
    string route_ref;
    int price_vnd = 0;
    string direction;
    bool is_transfer = false; // true if this is a transfer edge between routes
};

/// Calculate edge weight based on selected criterion
inline double calculate_weight(const Edge& e, WeightCriteria c) {
    switch (c) {
        case WeightCriteria::DISTANCE:
            return e.distance_km;
        case WeightCriteria::TIME:
            // Average bus speed ~25 km/h, return hours
            return e.distance_km / 25.0;
        case WeightCriteria::PRICE:
            return static_cast<double>(e.price_vnd);
        case WeightCriteria::TRANSFERS:
            return e.is_transfer ? 1.0 : 0.0;
    }
    return e.distance_km; // fallback
}

/// Convert criteria enum to display string
inline string criteria_to_string(WeightCriteria c) {
    switch (c) {
        case WeightCriteria::DISTANCE: return "distance (km)";
        case WeightCriteria::TIME: return "time (hours)";
        case WeightCriteria::PRICE: return "price (VND)";
        case WeightCriteria::TRANSFERS: return "transfers";
    }
    return "unknown";
}
