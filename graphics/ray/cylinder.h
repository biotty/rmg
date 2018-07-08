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

real_pair cylinder_intersection(const ray *, void * cylinder_);
direction cylinder_normal(point, void * cylinder_, bool at_second);

real_pair _cylinder_intersection(const ray *, void * cylinder_);
direction _cylinder_normal(point, void * cylinder_, bool at_second);

#ifdef __cplusplus
}
#endif

#endif
