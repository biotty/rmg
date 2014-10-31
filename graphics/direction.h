#ifndef DIRECTION_H
#define DIRECTION_H

#include "point.h"
#include <stdbool.h>

typedef xyz direction;

#define DISORIENTED ((direction){ .x = HUGE_REAL })
#define is_DISORIENTED(D) ((D)->x >= HUGE_REAL / 2)

#define point_from_origo(xyz) (xyz)
#define direction_from_origo(xyz) (xyz)
real length(direction);
void scale(direction *, real r);
void normalize(direction *);
real scalar_product(direction, direction);
direction reflection(direction normal, direction);
void move(point *, direction displacement);
direction distance_vector(point from, point to);
direction refraction(direction normal, direction, real w, real minimum_det);
void spherical(direction, real * r, real * theta, real * phi);
void direction_to_unitsquare(const direction * d, real * x, real * y);
direction rotation(direction d, real phi, real theta);
direction inverse_rotation(direction d, real phi, real theta);

#endif

