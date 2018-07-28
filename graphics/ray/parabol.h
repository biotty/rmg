//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef PARABOL_H
#define PARABOL_H

#include "ray.h"

struct parabol {
    direction translate;
    float r;
    tilt_arg rota;
};

#ifndef __cplusplus
typedef struct parabol parabol;
#endif

#ifdef __cplusplus
extern "C" {
#endif

segment parabol_intersection(const ray *, const void * parabol_, int * hit);
direction parabol_normal(point, const void * parabol_, int hit);

segment _parabol_intersection(const ray *, const void * parabol_, int * hit);
direction _parabol_normal(point, const void * parabol_, int hit);

#ifdef __cplusplus
}
#endif

#endif
