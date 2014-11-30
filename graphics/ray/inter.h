//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef INTER_H
#define INTER_H

#include "plane.h"
#include "sphere.h"
#include "cone.h"
#include "cylinder.h"
#include "scene.h"

typedef union {
    sphere sphere;
    plane plane;
    cone cone;
    cylinder cylinder;
} object_arg_union;

typedef void * (*object_generator)
    (object_intersection * fi, object_normal * fn);

void * new_inter(object_intersection * fi, object_normal * fn,
        int n, object_generator);

#endif

