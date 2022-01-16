#pragma once

#include <cmath>

// For Earth
// a = 6378137.0 m
// b = 6356752.314.245 m
// 1/f = 298.257223563

double Vincenty_Distance( const double& latitude_01, const double& longitude_01,
                          const double& latitude_02, const double& longitude_02,
                          const double& a,
                          const double& b );