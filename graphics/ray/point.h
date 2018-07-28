//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef POINT_H
#define POINT_H

#include "real.h"

struct point { real x, y, z; };

#ifndef __cplusplus
typedef struct point point;
#endif

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
