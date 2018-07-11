//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "hyperbol.h"


    real_pair
_hyperbol_intersection(
        const ray * ray_,
        const void * hyperbol__,
        int * hit)
{
    (void)hit;
    ray t = *ray_;
    const hyperbol * hyperbol_ = hyperbol__;
    move(&t.endpoint, hyperbol_->translate);
    inverse_rotation_ray(&t, hyperbol_->rota);
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
        const void * hyperbol__,
        int * hit)
{
    return invert(_hyperbol_intersection(ray_, hyperbol__, hit));
}

    direction
hyperbol_normal(point p, const void * hyperbol__, int hit)
{
    (void)hit;
    const hyperbol * hyperbol_ = hyperbol__;
    move(&p, hyperbol_->translate);
    direction n = inverse_rotation(direction_from_origo(p), hyperbol_->rota);
    n.z *= - hyperbol_->inv_v / hyperbol_->inv_h;
    normalize(&n);

    return rotation(n, hyperbol_->rota);
}

    direction
_hyperbol_normal(point p, const void * hyperbol__, int hit)
{
    direction d = hyperbol_normal(p, hyperbol__, hit);
    scale(&d, -1);
    return d;
}
