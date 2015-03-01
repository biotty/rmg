//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef RAY_H
#define RAY_H

#include "point.h"
#include "direction.h"

struct ray {
	point endpoint;
	direction head;
};

#ifndef __cplusplus
typedef struct ray ray;
#endif

void advance(ray * ray_, real r);

#endif

