//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef COLOR_H
#define COLOR_H

#include "real.h"
#include <stdbool.h>

struct color {  // linear
	real r;
	real g;
	real b;
};

struct compact_color {  // gamma-corrected values
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

#ifndef __cplusplus
typedef struct color color;
typedef struct compact_color compact_color;
#endif

static inline void saturated_add(compact_color * x, compact_color y)
{
    const int r_ = x->r + y.r;
    const int g_ = x->g + y.g;
    const int b_ = x->b + y.b;
    x->r = r_ < 256 ? r_ : 255;
    x->g = g_ < 256 ? g_ : 255;
    x->b = b_ < 256 ? b_ : 255;
}

static inline bool similar(double esq, const color * x, const color * y)
{
    if ( ! esq) {
        return x->r == y->r && x->r == y->r && x->r == y->r;
    } else {
        color w = { x->r - y->r, x->g - y->g, x->b - y->b };
        return esq > w.r * w.r + w.g * w.g + w.b * w.b;
    }
}

static inline real dec_stimulus(unsigned char c)
{
    const real s = c / 255.0;

    if (s <= 0.04045) return s / 12.92;
    else return rpow(((s + 0.055) / 1.055), 2.4);
}

static inline unsigned char enc_stimulus(real s)
{
    if (s <= 0.0031308) s *= 12.92;
    else s = rpow(s, 1.0 / 2.4) * 1.055 - 0.055;

    return (unsigned char)nearest(s * 255);
}

static inline color x_color(compact_color cc)
{
    color ret = {
        dec_stimulus(cc.r),
        dec_stimulus(cc.g),
        dec_stimulus(cc.b)
    };
    return ret;
}

static inline compact_color z_color(color c)
{
    compact_color ret = {
        enc_stimulus(c.r),
        enc_stimulus(c.g),
        enc_stimulus(c.b)
    };
    return ret;
}

// note: internal representation assumed by filter
//       is linear, so do not gamma-correct values
static inline compact_color z_filter(color c)
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
color from_hsv(real h, real s, real v);

#ifdef __cplusplus
}
#endif

#endif
