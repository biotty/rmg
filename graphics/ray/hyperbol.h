//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef HYPERBOL_H
#define HYPERBOL_H

#include "ray.h"

struct hyperbol {
    direction translate;
    float theta;
    float phi;
    float inv_h;
    float inv_v;
};

#ifndef __cplusplus
typedef struct hyperbol hyperbol;
#endif

#ifdef __cplusplus
extern "C" {
#endif

real_pair hyperbol_intersection(const ray *, void * hyperbol_);
direction hyperbol_normal(point, void * hyperbol_, bool at_second);

real_pair _hyperbol_intersection(const ray *, void * hyperbol_);
direction _hyperbol_normal(point, void * hyperbol_, bool at_second);

#ifdef __cplusplus
}
#endif

#endif
