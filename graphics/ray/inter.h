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

void init_arg_pool(int n, int i, int m);
void fini_arg_pool();
void * arg_alloc(size_t s);

void * make_inter(object_intersection * fi, object_normal * fn,
        int m, object_generator get);

#ifdef __cplusplus
}
#endif

#endif
