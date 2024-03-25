#pragma once
#include <limits>
#include <cstdlib>


// Constants
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;


// Utility Functions
inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

template<typename T>
inline T lerp(T start, T end, double t)
{
    return (1.0 - t) * start + t * end;
}

inline double random_double()
{
    return rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max)
{
    return min + (max - min) * random_double();
}