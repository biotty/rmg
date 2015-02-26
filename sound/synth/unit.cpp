//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "unit.hpp"
#include <algorithm>

namespace {

inline double linear(double a, double b, double r)
{
    return a * (1 - r) + b * r;
}

}

void unit::set(double w)
{
    std::fill(std::begin(y), std::end(y), w);
}

void unit::add(unit & u, double w)
{
    std::transform(std::begin(u.y), std::end(u.y),
            std::begin(y), std::begin(y),
            [w](double p, double q){
            return q + p * w;
            });
}

void unit::mul(unit & u, double w)
{
    const double _w = 1 - w;
    std::transform(std::begin(u.y), std::end(u.y),
            std::begin(y), std::begin(y),
            [_w](double p, double q){
            return q * linear(_w, 1, p);
            });
}
