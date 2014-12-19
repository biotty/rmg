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
#endif

void filter(color * light, color surface);
color optical_sum(color q, color w);
real intensity(color);

#ifdef __cplusplus
}
#endif
#endif

