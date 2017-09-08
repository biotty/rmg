//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef PARABOL_H
#define PARABOL_H

#include "ray.h"

struct parabol {
    direction translate;
    float r;
    float theta;
    float phi;
};

#ifndef __cplusplus
typedef struct parabol parabol;
#endif

#ifdef __cplusplus
extern "C" {
#endif

real_pair parabol_intersection(const ray *, void * parabol_);
direction parabol_normal(point, void * parabol_, bool at_second);

real_pair _parabol_intersection(const ray *, void * parabol_);
direction _parabol_normal(point, void * parabol_, bool at_second);

#ifdef __cplusplus
}
#endif

#endif
