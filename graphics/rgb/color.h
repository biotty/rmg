//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef COLOR_H
#define COLOR_H

#include "real.h"

typedef struct Color {
	real r;
	real g;
	real b;
} color;

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

static inline bool similar(double e, const color * a, const color * b)
{
    color d = { fabs(a->r - b->r), fabs(a->g - b->g), fabs(a->b - b->b) };
    return e * e > d.r * d.r + d.g * d.g + d.b * d.b;
}

void filter(color * light, color surface);
color optical_sum(color q, color w);
real intensity(color);

#ifdef __cplusplus
}
#endif
#endif

