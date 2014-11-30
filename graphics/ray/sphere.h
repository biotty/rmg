//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef SPHERE_H
#define SPHERE_H

#include "ray.h"

typedef struct {
	point center;
	real radius;
} sphere;

pair sphere_intersection(const ray *, void * sphere_);
direction sphere_normal(point, void * sphere_, bool at_second);

pair minusphere_intersection(const ray *, void * sphere_);
direction minusphere_normal(point, void * sphere_, bool at_second);

#endif

