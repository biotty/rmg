//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef DIRECTION_H
#define DIRECTION_H

#include "point.h"
#include <stdbool.h>

typedef xyz direction;

#define DISORIENTED ((direction){ .x = HUGE_REAL })
#define is_DISORIENTED(D) ((D)->x == HUGE_REAL)

#define point_from_origo(xyz) (xyz)
#define direction_from_origo(xyz) (xyz)

#ifdef __cplusplus
extern "C" {
#endif

    static inline real
length(direction direction_)
{
    return distance_to_origo(point_from_origo(direction_));
}

    static inline void
scale(direction * direction_, const real r)
{
    direction_->x *= r;
    direction_->y *= r;
    direction_->z *= r;
}

    static inline void
normalize(direction * direction_)
{
    scale(direction_, 1 / length(*direction_));
}

    static inline real
scalar_product(direction a, direction b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

    static inline void
move(point * p, direction displacement)
{
    p->x += displacement.x;
    p->y += displacement.y;
    p->z += displacement.z;
}

    static inline direction
distance_vector(point from, point to)
{
    const direction r = {
        to.x - from.x,
        to.y - from.y,
        to.z - from.z
    };
    return r;
}

direction reflection(direction normal, direction);
direction refraction(direction normal, direction, real w, real minimum_det);
void spherical(direction, real * r, real * theta, real * phi);
void direction_to_unitsquare(const direction * d, real * x, real * y);
direction rotation(direction d, real phi, real theta);
direction inverse_rotation(direction d, real phi, real theta);

#ifdef __cplusplus
}
#endif

#endif
