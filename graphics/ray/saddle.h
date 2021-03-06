//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef SADDLE_H
#define SADDLE_H

#include "ray.h"

struct saddle {
    direction translate;
    base_arg base;
    float h;
};

#ifndef __cplusplus
typedef struct saddle saddle;
#endif

#ifdef __cplusplus
extern "C" {
#endif

segment saddle_intersection(const ray *, const void * saddle_, int * hit);
direction saddle_normal(point, const void * saddle_, int hit);

#ifdef __cplusplus
}
#endif

#endif
