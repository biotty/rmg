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
#define TINY_REAL 0.000001
#define REAL_PI 3.1415926535897932
#define rabs fabs
#define rmod fmod
#define rpow pow
#define rfloor floor
#define ratan atan2
#define nearest round
#define sqroot sqrt

static inline real square(real r) { return r * r; }
static inline void mod1(real * x) {
    *x -= rfloor(*x);
    if (*x == 1) *x = 0;
}
static inline real max(real a, real b) { if (a > b) return a; else return b; }
static inline real min(real a, real b) { if (a < b) return a; else return b; }
static inline bool is_near(real a, real b) { return rabs(a - b) < 32*TINY_REAL; }

struct segment {
    real entry;
    real exit_;
};

#ifndef __cplusplus
typedef struct segment segment;
#endif

    static inline segment
invert(const segment p)
{
    segment r = { p.entry, p.entry };
    if (p.exit_ < 0) {
        assert(p.entry < 0);
        r.exit_ = HUGE_REAL;
        return r;
    }

    r.entry = p.exit_ == HUGE_REAL ? -1 : p.exit_;
    return r;
}

    static inline segment
quadratic(const real a, const real b, const real c)
{
    segment r = { -1, HUGE_REAL };
    if ((a >= 0) == (b >= 0)) {
        if ((a >= 0) == (c >= 0)) {
            if (a <= 0) r.exit_ = -1;
            return r;
        }
    }
    if (a == 0) {
        const real q = - c / b;
        if (b <= 0) r.exit_ = q;
        else r.entry = q;
        return r;
    }
    const real det = square(b) - 4 * a * c;
    if (det <= 0) {
        if (a <= 0) r.exit_ = -1;
        return r;
    }
    const real sqrt_ = sqroot(det)
        * (a < 0 ? -1 : 1);
    const real f = 0.5 / a;
    const real t2 = f * (-b + sqrt_);
    if (t2 <= 0) {
        if (a <= 0) r.exit_ = -1;
        return r;
    }
    real t1 = f * (-b - sqrt_);
    if (t1 < 0) {
        if (a <= 0) r.exit_ = t2;
        else r.entry = t2;
        return r;
    }
    if (a <= 0) {
        r.entry = t1;
        r.exit_ = t2;
    } else {
        r.entry = t2;
        r.exit_ = t1;
    }
    return r;
}

#endif
