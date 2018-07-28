//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef HYPERBOL_H
#define HYPERBOL_H

#include "ray.h"

struct hyperbol {
    direction translate;
    tilt_arg rota;
    float inv_h;
    float inv_v;
};

#ifndef __cplusplus
typedef struct hyperbol hyperbol;
#endif

#ifdef __cplusplus
extern "C" {
#endif

segment hyperbol_intersection(const ray *, const void * hyperbol_, int * hit);
direction hyperbol_normal(point, const void * hyperbol_, int hit);

segment _hyperbol_intersection(const ray *, const void * hyperbol_, int * hit);
direction _hyperbol_normal(point, const void * hyperbol_, int hit);

#ifdef __cplusplus
}
#endif

#endif
