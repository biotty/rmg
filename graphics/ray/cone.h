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

real_pair cone_intersection(const ray *, void * cone_);
direction cone_normal(point, void * cone_, bool at_second);

real_pair _cone_intersection(const ray *, void * cone_);
direction _cone_normal(point, void * cone_, bool at_second);

#ifdef __cplusplus
}
#endif

#endif
