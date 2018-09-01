//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef DIRECTION_H
#define DIRECTION_H

#include "point.h"
#include <stdbool.h>

struct direction { real x, y, z; };

#ifndef __cplusplus
typedef struct direction direction;
#endif

#define DISORIENTED ((direction){ .x = HUGE_REAL })
#define is_DISORIENTED(D) ((D)->x == HUGE_REAL)

    static inline point
point_from_origo(direction d)
{
    point p = {d.x, d.y, d.z};
    return p;
}

    static inline direction
direction_from_origo(point p)
{
    direction d = {p.x, p.y, p.z};
    return d;
}

struct tilt_arg
{
    float theta_cos;
    float theta_sin;
    float phi_cos;
    float phi_sin;
};

struct base_arg
{
    direction x;
    direction z;
};

#ifndef __cplusplus
typedef struct tilt_arg tilt_arg;
typedef struct base_arg base_arg;
#else
extern "C" {
#endif

    static inline real
length(const direction direction_)
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
direction refraction(direction normal, direction, real w);
void spherical(direction, real * r, real * theta, real * phi);
void spherical_arg(direction d, real * r, tilt_arg * arg);
void direction_to_unitsquare(const direction * d, real * x, real * y);
direction tilt(direction d, tilt_arg arg);
direction inverse_tilt(direction d, tilt_arg arg);
direction cross(direction a, direction b);
direction base(direction d, base_arg arg);
direction inverse_base(direction d, base_arg arg);

#ifdef __cplusplus
}
#endif

#endif
