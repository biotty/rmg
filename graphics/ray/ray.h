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
void inverse_tilt_ray(ray * ray_, tilt_arg arg);
void inverse_base_ray(ray * ray_, base_arg arg);

#ifdef __cplusplus
}
#endif

#endif

