//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "cylinder.h"
#include <assert.h>


    static direction
transform_(point p, const cylinder * cylinder_)
{
    move(&p, cylinder_->translate);
    return inverse_rotation(direction_from_origo(p), cylinder_->rota);
}

    static void
transform_ray_(ray * ray_, const cylinder * cylinder_)
{
    move(&ray_->endpoint, cylinder_->translate);
    inverse_rotation_ray(ray_, cylinder_->rota);
}

    real_pair
_cylinder_intersection(
        const ray * ray_,
        void * cylinder__)
{
    ray t = *ray_;
    const cylinder * cylinder_ = cylinder__;
    transform_ray_(&t, cylinder_);

    const real h_x = t.head.x;
    const real h_y = t.head.y;
    const real e_x = t.endpoint.x;
    const real e_y = t.endpoint.y;
    const real sq_r = cylinder_->r;
    const real sq_e = square(e_x) + square(e_y);
    const real sq_h = square(h_x) + square(h_y);
    if (sq_h == 0) {
        return (real_pair){-1, sq_e > sq_r ? HUGE_REAL : -1};
    }
    const real a = sq_h;
    const real b = 2 * (e_x * h_x + e_y * h_y);
    const real c = sq_e - sq_r;
    return quadratic(a, b, c);
}

real_pair
cylinder_intersection(
        const ray * ray_,
        void * cylinder__)
{
    return invert(_cylinder_intersection(ray_, cylinder__));
}

    direction
cylinder_normal(point p, void * cylinder__, bool at_second)
{
    (void)at_second;
    const cylinder * cylinder_ = cylinder__;
    direction n = transform_(p, cylinder_);
    
    n.z = 0;
    normalize(&n);

    return rotation(n, cylinder_->rota);
}

    direction
_cylinder_normal(point p, void * cylinder__, bool at_second)
{
    direction d = cylinder_normal(p, cylinder__, at_second);
    scale(&d, -1);
    return d;
}
