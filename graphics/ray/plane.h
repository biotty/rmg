//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef PLANE_H
#define PLANE_H

#include "ray.h"

struct plane {
    float at_x;
    float at_y;
    float at_z;
    direction normal;
};

#ifndef __cplusplus
typedef struct plane plane;
#endif

#ifdef __cplusplus
extern "C" {
#endif

real_pair plane_intersection(const ray *, void * plane_);
direction plane_normal(point, void * plane_, bool at_second);

#ifdef __cplusplus
}
#endif

#endif
