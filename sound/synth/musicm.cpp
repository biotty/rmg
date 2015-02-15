//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "musicm.hpp"
#include <stdexcept>
#include <cstdlib>
#include <cassert>
#include <cmath>

namespace {

double just_ratio_firsthalf(unsigned i)
{
    switch (i) {
    case 0: return 1;
    case 1: return 16/15.0;
    case 2: return 9/8.0;
    case 3: return 6/5.0;
    case 4: return 5/4.0;
    case 5: return 4/3.0;
    case 6: return 7/5.0;
    }
    throw std::runtime_error("not reached");
}

double just_ratio(unsigned i)
{
    assert (i < 12);
    return (i < 7)
        ? just_ratio_firsthalf(i)
        : 2 / just_ratio_firsthalf(12 - i);
}

} //namespace

double p_of(double f) { return 69 + 12 * std::log2(f / 440); }
double c_at(double f) { return f_of(p_of(f) + .01) - f; }
double a_of(double l) { return std::pow(10, l / 20); }
double l_of(double a) { return 20 * std::log10(a); }
double f_of(double p) // equal-tempered
{
    return 440 * std::pow(2, (p - 69) / 12);
}

double f_of_just(unsigned p)
{
    div_t d = div(p, 12);
    int & o = d.quot;
    int & i = d.rem;
    const double f = 8.175798915643707; // ::f_of(0);
    return f * std::pow(2, o) * just_ratio(i);
}
