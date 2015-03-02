//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef INTER_H
#define INTER_H

#include "plane.h"
#include "sphere.h"
#include "cone.h"
#include "cylinder.h"
#include "scene.h"

union object_arg_union {
    sphere sphere_;
    plane plane_;
    cone cone_;
    cylinder cylinder_;
};

typedef union object_arg_union (*object_generator)
    (object_intersection * fi, object_normal * fn);

#ifndef __cplusplus
typedef union object_arg_union object_arg_union;
#endif

#ifdef __cplusplus
extern "C" {
#endif

void * new_inter(object_intersection * fi, object_normal * fn,
        int n, object_generator);

object_arg_union * new_object_arg();
void delete_object_or_inter(void *);

#ifdef __cplusplus
}
#endif

#endif
