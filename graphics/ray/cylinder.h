//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef CYLINDER_H
#define CYLINDER_H

#include "ray.h"

struct cylinder {
    direction translate;
    float r;
    rotation_arg rota;
};

#ifndef __cplusplus
typedef struct cylinder cylinder;
#endif

#ifdef __cplusplus
extern "C" {
#endif

segment cylinder_intersection(const ray *, const void * cylinder_, int * hit);
direction cylinder_normal(point, const void * cylinder_, int hit);

segment _cylinder_intersection(const ray *, const void * cylinder_, int * hit);
direction _cylinder_normal(point, const void * cylinder_, int hit);

#ifdef __cplusplus
}
#endif

#endif
