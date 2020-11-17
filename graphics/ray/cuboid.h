//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef CUBOID_H
#define CUBOID_H

#include "ray.h"

struct cuboid {
    direction translate;
    base_arg base;
    direction s;
};

#ifndef __cplusplus
typedef struct cuboid cuboid;
#endif

#ifdef __cplusplus
extern "C" {
#endif

segment cuboid_intersection(const ray *, const void * cuboid_, int * hit);
direction cuboid_normal(point, const void * cuboid_, int hit);

segment _cuboid_intersection(const ray *, const void * cuboid_, int * hit);
direction _cuboid_normal(point, const void * cuboid_, int hit);

#ifdef __cplusplus
}
#endif

#endif
