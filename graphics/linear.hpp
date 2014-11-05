//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef LINEAR_HPP
#define LINEAR_HPP

#include <cstdlib>


static inline double rnd(double f) { return std::rand() * f / RAND_MAX; }


template<typename T>
T linear(T a, T b, double k)
{
    return a * (1 - k)
        + b * k;
}


struct Bounds {
    double lower;
    double upper;
    static const double lower_unset;
    static const double upper_unset;

    Bounds() : lower(lower_unset), upper(upper_unset) {}
    void update(double v)
    {
        if (v < lower) lower = v;
        if (v > upper) upper = v;
    }
    double span() { return upper - lower; }
    double factor() { double s = span(); return s ? 1 / s : 1; }
};

    
const double Bounds::lower_unset = 1e9;
const double Bounds::upper_unset = -1e9;

#endif

