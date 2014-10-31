#ifndef MATH_H
#define MATH_H

#include "real.h"
#include <stdbool.h>

static inline real square(real r) { return r * r; }
static inline real zoom(const real r, real x) {
    x -= rfloor(x);
    return (x == 1) ? 0 : x;
}
static inline real max(real a, real b) { if (a > b) return a; else return b; }
static inline real min(real a, real b) { if (a < b) return a; else return b; }
static inline bool is_near(real a, real b) { return rabs(a - b) < 32*TINY_REAL; }
static inline bool is_pretty_near(real a, real b) { return rabs(a - b) < 0.001; }


#endif

