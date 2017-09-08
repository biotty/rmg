//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef SADDLE_H
#define SADDLE_H

#include "ray.h"

struct saddle {
    direction translate;
    float theta;
    float phi;
    float v;
    point scale;
};

#ifndef __cplusplus
typedef struct saddle saddle;
#endif

#ifdef __cplusplus
extern "C" {
#endif

real_pair saddle_intersection(const ray *, void * saddle_);
direction saddle_normal(point, void * saddle_, bool at_second);

#ifdef __cplusplus
}
#endif

#endif
