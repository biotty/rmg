//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef PLANE_H
#define PLANE_H

#include "ray.h"

struct plane {
    point at;
    direction normal;
};

#ifndef __cplusplus
typedef struct plane plane;
#endif

#ifdef __cplusplus
extern "C" {
#endif

segment axis_plane_intersection(double a, double b);
segment plane_intersection(const ray *, const void * plane_, int * hit);
direction plane_normal(point, const void * plane_, int hit);

#ifdef __cplusplus
}
#endif

#endif
