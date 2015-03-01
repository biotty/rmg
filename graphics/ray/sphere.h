//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef SPHERE_H
#define SPHERE_H

#include "ray.h"

struct sphere {
	point center;
	real radius;
};

#ifndef __cplusplus
typedef struct sphere sphere;
#endif

#ifdef __cplusplus
extern "C" {
#endif

real_pair sphere_intersection(const ray *, void * sphere_);
direction sphere_normal(point, void * sphere_, bool at_second);

real_pair minusphere_intersection(const ray *, void * sphere_);
direction minusphere_normal(point, void * sphere_, bool at_second);

#ifdef __cplusplus
}
#endif

#endif
