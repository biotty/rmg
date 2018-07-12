//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "sphere.h"
#include <math.h>
#include <assert.h>

segment
_sphere_intersection(
        const ray * ray_,
        const void * sphere__,
        int * hit)
{
    (void)hit;
    const sphere * sphere_ = sphere__;
    const direction d = distance_vector(sphere_->center, ray_->endpoint);
    const real b = 2 * scalar_product(d, ray_->head);
    const real c = square(d.x) + square(d.y) + square(d.z)
        - sphere_->sq_radius;
    return quadratic(1, b, c);
}

segment
sphere_intersection(
        const ray * ray_,
        const void * sphere__,
        int * hit)
{
    return invert(_sphere_intersection(ray_, sphere__, hit));
}

    direction
sphere_normal(point p, const void * sphere__, int hit)
{
    (void)hit;
    const sphere * sphere_ = sphere__;
    direction translate = sphere_->center;
    scale(&translate, -1);
    move(&p, translate);
    direction d = direction_from_origo(p);
    normalize(&d);
    return d;
}

    direction
_sphere_normal(point p, const void * sphere__, int hit)
{
    direction d = sphere_normal(p, sphere__, hit);
    scale(&d, -1);
    return d;
}

