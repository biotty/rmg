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
#else
extern "C" {
#endif

void advance(ray * ray_, real r);
void inverse_rotation_ray(ray * ray_, real phi, real theta);

#ifdef __cplusplus
}
#endif

#endif

