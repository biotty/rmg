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

struct compact_color {  // gamma-corrected values (except when for filter)
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
    unsigned int r_ = x->r + y.r;
    unsigned int g_ = x->g + y.g;
    unsigned int b_ = x->b + y.b;
    x->r = r_ < 256 ? r_ : 255;
    x->g = g_ < 256 ? g_ : 255;
    x->b = b_ < 256 ? b_ : 255;
}

static inline bool similar(double esq, const color * x, const color * y)
{
    if ( ! esq) {
        return x->r == y->r && x->g == y->g && x->b == y->b;
    }

    color w = { x->r - y->r, x->g - y->g, x->b - y->b };
    return esq > w.r * w.r + w.g * w.g + w.b * w.b;
}

static inline real dec_stimulus(unsigned char c)
{
    const real s = c / 255.0;
    if (s <= 0.04045) {
        return s / 12.92;
    }

    return rpow(((s + 0.055) / 1.055), 2.4);
}

static inline real stimulus_apply_gamma(real s)
{
    if (s <= 0.0031308) s *= 12.92;
    else s = rpow(s, 1.0 / 2.4) * 1.055 - 0.055;

    return s;
}

static inline void enc_hifi_stimulus(real s, unsigned char * a)
{
    unsigned int v = stimulus_apply_gamma(s) * 65535;
    a[0] = v >> 8;
    a[1] = v;
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
        (unsigned char)nearest(stimulus_apply_gamma(c.r) * 255),
        (unsigned char)nearest(stimulus_apply_gamma(c.g) * 255),
        (unsigned char)nearest(stimulus_apply_gamma(c.b) * 255)
    };
    return ret;
}

static inline void zz_color(color c, unsigned char * enc)
{
    enc_hifi_stimulus(c.r, enc + 0);
    enc_hifi_stimulus(c.g, enc + 2);
    enc_hifi_stimulus(c.b, enc + 4);
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
