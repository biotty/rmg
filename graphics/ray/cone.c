//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "cone.h"


    static direction
transform_(point p, const cone * cone_)
{
    move(&p, cone_->translate);
    direction endpoint = inverse_tilt(direction_from_origo(p), cone_->rota);
    return endpoint;
}

    static void
transform_ray_(ray * ray_, const cone * cone_)
{
    move(&ray_->endpoint, cone_->translate);
    inverse_tilt_ray(ray_, cone_->rota);
}

    segment
cone_intersection(
        const ray * ray_,
        const void * cone__,
        int * hit)
{
    (void)hit;
    ray t = *ray_;
    const cone * cone_ = cone__;
    transform_ray_(&t, cone_);
    t.endpoint.z *= cone_->inv_r;
    t.head.z *= cone_->inv_r;

    const real sq_absxy = square(t.endpoint.x) + square(t.endpoint.y);
    const real a = square(t.head.z) - square(t.head.x) - square(t.head.y);
    if (a == 0) { // parallel to some line on the surface
        const real sqrt2 = 1.4142135623730950488016887242096981L;
        const real absxy = sqrt(sq_absxy);
        const real z = t.endpoint.z;
        if (z < -absxy) return (segment){-1, sqrt2 * (-z)};
        if (z == absxy) return (segment){-1, -1};
        if (z <= absxy) return (segment){(1/sqrt2) * (absxy - z), HUGE_REAL};
        return                 (segment){-1, HUGE_REAL};
    }
    const real b = 2 * (t.endpoint.z * t.head.z - t.endpoint.x * t.head.x - t.endpoint.y * t.head.y);
    const real c = square(t.endpoint.z) - sq_absxy;
    // positive a means head intersects (out, in); has z greater than xy-plane projection
    // positive b means head counters normal; along hyperbolic field that enters surface
    // positive c means endpoint is inside
    return quadratic(a, b, c);
}

    segment
_cone_intersection(
        const ray * ray_,
        const void * cone__,
        int * hit)
{
    return invert(cone_intersection(ray_, cone__, hit));
}

    direction
cone_normal(point p, const void * cone__, int hit)
{
    (void)hit;
    const cone * cone_ = cone__;
    direction n = transform_(p, cone_);

    n.z *= - square(cone_->inv_r);
    normalize(&n);

    return tilt(n, cone_->rota);
}

    direction
_cone_normal(point p, const void * cone__, int hit)
{
    direction d = cone_normal(p, cone__, hit);
    scale(&d, -1);
    return d;
}

