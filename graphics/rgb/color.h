//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef COLOR_H
#define COLOR_H

#include "real.h"
#include <stdbool.h>

struct color {
	real r;
	real g;
	real b;
};

#ifndef __cplusplus
typedef struct color color;
#endif

static inline bool similar(double e, const color * a, const color * b)
{
    color d = { fabs(a->r - b->r), fabs(a->g - b->g), fabs(a->b - b->b) };
    return e * e > d.r * d.r + d.g * d.g + d.b * d.b;
}

#ifdef __cplusplus
extern "C" {
#endif

void filter(color * light, color surface);
color optical_sum(color q, color w);
real intensity(color);

#ifdef __cplusplus
}
#endif

#endif
