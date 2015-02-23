//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef UNIT_HPP
#define UNIT_HPP

#include "unitfwd.hpp"
#include <memory>

#define SU 441

#define FOR(X, A, B) for (unsigned X=A; X!=B; ++X)
#define FOR_SU(X) FOR(X, 0, SU)

struct unit
{
    double y[SU];

    void set(double s);
    void add(unit & u, double w);
    void mul(unit & u, double w);
};

#endif
