//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef REAL_H
#define REAL_H

#include <math.h>
#include <float.h>
#include <stdbool.h>
#include <assert.h>

typedef double real;
#define REAL_FMT "%lf"
#define REAL_EPSILON DBL_EPSILON
#define HUGE_REAL HUGE_VAL
#define TINY_REAL 0.00001
#define REAL_PI 3.1415926535897932
static inline real rabs(real r) { return fabs(r); }
static inline real rmod(real r, real s) { return fmod(r, s); }
static inline real rpow(real r, real e) { return pow(r, e); }
static inline real rfloor(real r) { return floor(r); }
static inline real ratan(real y, real x) { return atan2(y, x); }
static inline int nearest(real r) { return round(r); }
static inline real sqroot(real r) { return sqrt(r); }

static inline real square(real r) { return r * r; }
static inline void mod1(real * x) {
    *x -= rfloor(*x);
    if (*x == 1) *x = 0;
}
static inline real max(real a, real b) { if (a > b) return a; else return b; }
static inline real min(real a, real b) { if (a < b) return a; else return b; }
static inline bool is_near(real a, real b) { return rabs(a - b) < 32*TINY_REAL; }

struct real_pair {
    real first;
    real second;
};

#ifndef __cplusplus
typedef struct real_pair real_pair;
#endif

    static inline real_pair
invert(const real_pair p)
{
    real_pair r = { p.first, p.first };
    if (p.second < 0) {
        assert(p.first < 0);
        r.second = HUGE_REAL;
        return r;
    }

    r.first = p.second == HUGE_REAL ? -1 : p.second;
    return r;
}

    static inline real_pair
quadratic(const real a, const real b, const real c)
{
    real_pair r = { -1, HUGE_REAL };
    if ((a >= 0) == (b >= 0)) {
        if ((a >= 0) == (c >= 0)) {
            if (a <= 0) r.second = -1;
            return r;
        }
    }
    if (a == 0) {
        const real q = - c / b;
        if (b <= 0) r.second = q;
        else r.first = q;
        return r;
    }
    const real det = square(b) - 4 * a * c;
    if (det <= 0) {
        if (a <= 0) r.second = -1;
        return r;
    }
    const real sqrt_ = sqroot(det)
        * (a < 0 ? -1 : 1);
    const real f = 0.5 / a;
    const real t2 = f * (-b + sqrt_);
    if (t2 <= 0) {
        if (a <= 0) r.second = -1;
        return r;
    }
    real t1 = f * (-b - sqrt_);
    if (t1 < 0) {
        if (a <= 0) r.second = t2;
        else r.first = t2;
        return r;
    }
    if (a <= 0) {
        r.first = t1;
        r.second = t2;
    } else {
        r.first = t2;
        r.second = t1;
    }
    return r;
}

#endif
