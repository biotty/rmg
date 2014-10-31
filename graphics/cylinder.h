#ifndef CYLINDER_H
#define CYLINDER_H

#include "ray.h"

typedef struct {
	direction translate;
    real r;
    real theta;
    real phi;
} cylinder;

pair cylinder_intersection(const ray *, void * cylinder_);
direction cylinder_normal(point, void * cylinder_, bool at_second);

pair minucylinder_intersection(const ray *, void * cylinder_);
direction minucylinder_normal(point, void * cylinder_, bool at_second);

#endif

