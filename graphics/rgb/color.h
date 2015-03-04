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

struct compact_color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

#ifndef __cplusplus
typedef struct color color;
typedef struct compact_color compact_color;
#endif

static inline bool similar(double e, const color * a, const color * b)
{
    color d = { fabs(a->r - b->r), fabs(a->g - b->g), fabs(a->b - b->b) };
    return e * e > d.r * d.r + d.g * d.g + d.b * d.b;
}

static inline color x_color(compact_color cc)
{
    color ret = {
        cc.r /(real) 255,
        cc.g /(real) 255,
        cc.b /(real) 255
    };
    return ret;
}

static inline compact_color z_color(color c)
{
    compact_color ret = {
        (unsigned char)nearest(c.r * 255),
        (unsigned char)nearest(c.g * 255),
        (unsigned char)nearest(c.b * 255)
    };
    return ret;
}

static inline compact_color str_color(unsigned char * rgb)
{
    compact_color ret = { rgb[0], rgb[1], rgb[2] };
    return ret;
}

#ifdef __cplusplus
extern "C" {
#endif

void filter(color * light, compact_color surface);
void color_add(color * q, color w);
real intensity(color);

#ifdef __cplusplus
}
#endif

#endif
