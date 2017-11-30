//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "hyperbol.h"


    real_pair
_hyperbol_intersection(
        const ray * ray_,
        void * hyperbol__)
{
    ray t = *ray_;
    const hyperbol * hyperbol_ = hyperbol__;
    move(&t.endpoint, hyperbol_->translate);
    inverse_rotation_ray(&t, hyperbol_->theta, hyperbol_->phi);
    t.endpoint.x *= hyperbol_->inv_v;
    t.endpoint.y *= hyperbol_->inv_v;
    t.endpoint.z *= hyperbol_->inv_h;
    t.head.x *= hyperbol_->inv_v;
    t.head.y *= hyperbol_->inv_v;
    t.head.z *= hyperbol_->inv_h;

    const real a = square(t.head.x) + square(t.head.y) - square(t.head.z);
    const real b = 2 * ( - t.endpoint.z * t.head.z
        + t.endpoint.x * t.head.x
        + t.endpoint.y * t.head.y);
    const real c = square(t.endpoint.x) + square(t.endpoint.y)
        - square(t.endpoint.z) - 1;
    return quadratic(a, b, c);
}

    real_pair
hyperbol_intersection(
        const ray * ray_,
        void * hyperbol__)
{
    return invert(_hyperbol_intersection(ray_, hyperbol__));
}

    direction
hyperbol_normal(point p, void * hyperbol__, bool at_second)
{
    (void)at_second;
    const hyperbol * hyperbol_ = hyperbol__;
    move(&p, hyperbol_->translate);
    direction n = inverse_rotation(direction_from_origo(p), hyperbol_->theta, hyperbol_->phi);
    n.z *= - hyperbol_->inv_v / hyperbol_->inv_h;
    normalize(&n);

    return rotation(n, hyperbol_->theta, hyperbol_->phi);
}

    direction
_hyperbol_normal(point p, void * hyperbol__, bool at_second)
{
    direction d = hyperbol_normal(p, hyperbol__, at_second);
    scale(&d, -1);
    return d;
}