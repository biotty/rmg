//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef SPHERE_H
#define SPHERE_H

#include "ray.h"

struct sphere {
	point center;
	real sq_radius;
};

#ifndef __cplusplus
typedef struct sphere sphere;
#endif

#ifdef __cplusplus
extern "C" {
#endif

segment sphere_intersection(const ray *, const void * sphere_, int * hit);
direction sphere_normal(point, const void * sphere_, int hit);

segment _sphere_intersection(const ray *, const void * sphere_, int * hit);
direction _sphere_normal(point, const void * sphere_, int hit);

#ifdef __cplusplus
}
#endif

#endif
