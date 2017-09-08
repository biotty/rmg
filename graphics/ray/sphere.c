//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "sphere.h"
#include <math.h>
#include <assert.h>

real_pair
_sphere_intersection(
        const ray * ray_,
        void * sphere__)
{
    const sphere * sphere_ = sphere__;
    const direction d = distance_vector(sphere_->center, ray_->endpoint);
    const real b = 2 * scalar_product(d, ray_->head);
    const real c = square(d.x) + square(d.y) + square(d.z)
        - sphere_->sq_radius;
    return quadratic(1, b, c);
}

real_pair
sphere_intersection(
        const ray * ray_,
        void * sphere__)
{
    return invert(_sphere_intersection(ray_, sphere__));
}

    direction
sphere_normal(point p, void * sphere__, bool at_second)
{
    (void)at_second;
    const sphere * sphere_ = sphere__;
    direction translate = sphere_->center;
    scale(&translate, -1);
    move(&p, translate);
    direction d = direction_from_origo(p);
    normalize(&d);
    return d;
}

    direction
_sphere_normal(point p, void * sphere__, bool at_second)
{
    direction d = sphere_normal(p, sphere__, at_second);
    scale(&d, -1);
    return d;
}

