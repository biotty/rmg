//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef CONE_H
#define CONE_H

#include "ray.h"

struct cone {
    direction translate;
    float inv_r;
    rotation_arg rota;
};

#ifndef __cplusplus
typedef struct cone cone;
#endif

#ifdef __cplusplus
extern "C" {
#endif

real_pair cone_intersection(const ray *, const void * cone_, int * hit);
direction cone_normal(point, const void * cone_, int hit);

real_pair _cone_intersection(const ray *, const void * cone_, int * hit);
direction _cone_normal(point, const void * cone_, int hit);

#ifdef __cplusplus
}
#endif

#endif
