//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "unit.hpp"

namespace {

inline double linear(double a, double b, double r)
{
    return a * (1 - r) + b * r;
}

}//namespace

void unit::set(double s)
{
    FOR_SU(i) y[i] = s;
}

void unit::add(unit & u, double w)
{
    FOR_SU(i) y[i] += u.y[i] * w;
}

void unit::mul(unit & u, double w)
{
    FOR_SU(i) y[i] *= linear(1 - w, 1, u.y[i]);
}

