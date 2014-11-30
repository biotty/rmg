//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef PLANE_H
#define PLANE_H

#include "ray.h"

typedef struct {
    point point;
    direction normal;
} plane;

pair plane_intersection(const ray *, void * plane_);
direction plane_normal(point, void * plane_, bool at_second);

#endif
