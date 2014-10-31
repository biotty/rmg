#ifndef CONE_H
#define CONE_H

#include "ray.h"

typedef struct {
	direction translate;
	real r;
    real theta;
    real phi;
} cone;

pair cone_intersection(const ray *, void * cone_);
direction cone_normal(point, void * cone_, bool at_second);

pair minucone_intersection(const ray *, void * cone_);
direction minucone_normal(point, void * cone_, bool at_second);

#endif

