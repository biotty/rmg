//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef REAL_H
#define REAL_H

#include <math.h>
#include <float.h>

#ifdef REAL_FLT

#ifdef REAL_LDBL
#error "Both REAL_FLT and REAL_LDBL defined"
#endif

#define REAL_TYPE float
typedef REAL_TYPE real;
#define REAL_FMT "%f"
#define REAL_EPSILON FLT_EPSILON
#define HUGE_REAL HUGE_VALF
#define TINY_REAL 0.0002
#define REAL_PI 3.14159265
static inline real rabs(real r) { return fabsf(r); }
static inline real rmod(real r, real s) { return fmodf(r, s); }
static inline real rpow(real r, real e) { return powf(r, e); }
static inline real rfloor(real r) { return floorf(r); }
static inline real ratan(real y, real x) { return atan2f(y, x); }
static inline int nearest(real r) { return roundf(r); }
static inline real sqroot(real r) { return sqrtf(r); }

#endif

#ifdef REAL_LDBL

#define REAL_TYPE long double
typedef REAL_TYPE real;
#define REAL_FMT "%Lf"
#define REAL_EPSILON LDBL_EPSILON
#define HUGE_REAL HUGE_VALL
#define TINY_REAL 0.0000001
#define REAL_PI 3.1415926535897932384626433832795l
static inline real rabs(real r) { return fabsl(r); }
static inline real rmod(real r, real s) { return fmodl(r, s); }
static inline real rpow(real r, real e) { return powl(r, e); }
static inline real rfloor(real r) { return floorl(r); }
static inline real ratan(real y, real x) { return atan2l(y, x); }
static inline int nearest(real r) { return roundl(r); }
static inline real sqroot(real r) { return sqrtl(r); }

#endif

#ifndef REAL_TYPE

#define REAL_TYPE double
typedef REAL_TYPE real;
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

#endif

struct real_pair {
    real first;
    real second;
};

#ifndef __cplusplus
typedef struct real_pair real_pair;
#endif

#endif
