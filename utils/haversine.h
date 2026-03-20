#pragma once

#include <cmath>

using namespace std;

/// Tinh khoang cach duong tron lon giua 2 toa do GPS
/// Tra ve khoang cach tinh bang km between two GPS coordinates using Haversine formula.
/// @return Distance in kilometers
inline double haversine(double lat1, double lng1, double lat2, double lng2) {
    constexpr double EARTH_RADIUS_KM = 6371.0;
    constexpr double DEG_TO_RAD = M_PI / 180.0;

    double d_lat = (lat2 - lat1) * DEG_TO_RAD;
    double d_lng = (lng2 - lng1) * DEG_TO_RAD;

    double a = sin(d_lat / 2) * sin(d_lat / 2) +
               cos(lat1 * DEG_TO_RAD) * cos(lat2 * DEG_TO_RAD) *
               sin(d_lng / 2) * sin(d_lng / 2);

    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
    return EARTH_RADIUS_KM * c;
}
