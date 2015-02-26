//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef UNIT_HPP
#define UNIT_HPP

#include "unitfwd.hpp"

struct unit
{
    static constexpr size_t size = 441;
    double y[size];

    void set(double s);
    void add(unit & u, double w);
    void mul(unit & u, double w);
};

#endif
