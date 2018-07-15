//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef POINT_H
#define POINT_H

#include "xyz.h"

typedef xyz point;

static const point origo = {0, 0, 0};

#ifdef __cplusplus
extern "C" {
#endif

real distance(point a, point b);
real distance_to_origo(point p);

#ifdef __cplusplus
}
#endif

#endif
