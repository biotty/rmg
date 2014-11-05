//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef RAY_H
#define RAY_H

#include "point.h"
#include "direction.h"

typedef struct {
	point endpoint;
	direction head;
} ray;

void advance(ray * ray_, real r);

#endif

