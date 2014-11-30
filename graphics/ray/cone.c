//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "cone.h"
#include "xmath.h"
#include <assert.h>


    static direction
transform_(point p, const cone * cone_)
{
    move(&p, cone_->translate);
    direction endpoint = inverse_rotation(direction_from_origo(p), cone_->theta, cone_->phi);
    endpoint.z *= cone_->r;
    return endpoint;
}

    pair
cone_intersection(
        const ray * ray_,
        void * cone__)
{
    const cone * cone_ = cone__;
    direction endpoint = transform_(ray_->endpoint, cone_);
    direction head = inverse_rotation(ray_->head, cone_->theta, cone_->phi);
    head.z *= cone_->r;

    const real sq_absxy = square(endpoint.x) + square(endpoint.y);
    const real a = square(head.z) - square(head.x) - square(head.y);
    if (a == 0) { // parallell to some line on the surface
        const real sqrt2 = 1.4142135623730950488016887242096981L;
        const real absxy = sqrt(sq_absxy);
        const real z = endpoint.z;
        if (z < -absxy) return (pair){-1, sqrt2 * (-z)};
        if (z == absxy) return (pair){-1, -1};
        if (z <= absxy) return (pair){(1/sqrt2) * (absxy - z), HUGE_REAL};
                        return (pair){-1, HUGE_REAL};
    }
    const real b = 2 * (endpoint.z * head.z - endpoint.x * head.x - endpoint.y * head.y);
    const real c = square(endpoint.z) - sq_absxy;
    // positive a means head intersects (out, in); has z stronger than xy-plane projection
    // positive b means head counters normal; along hyperbolic field that enters surface
    // positive c means endpoint is inside
    
    const real det = square(b) - 4 * a * c;
    if (det <= 0) return (pair){-1, -1};
    const real sqrt_= sqroot(det);
    const real f = 0.5 / a;
    real t1 = f * (-b - sqrt_);
    real t2 = f * (-b + sqrt_);
    if (a > 0) {
        if (t2 < 0) return (pair){-1, HUGE_REAL};
        if (t1 < 0) return (pair){t2, HUGE_REAL};
        return (pair){t2, t1};
    }
    return (pair){t2, t1};  // reverse because we have here have that f is negative
}

    pair
minucone_intersection(
        const ray * ray_,
        void * cone__)
{
    pair p = cone_intersection(ray_, cone__);
    if (p.first >= 0) {
        assert(p.second >= 0);
        return (pair){p.second, p.first};
    } else
        return (pair){p.second, HUGE_REAL};
}

    direction
cone_normal(point p, void * cone__, bool at_second)
{
    (void)at_second;
    const cone * cone_ = cone__;
    direction n = transform_(p, cone_);
    
    n.z *= - cone_->r;
    normalize(&n);

    return rotation(n, cone_->theta, cone_->phi);
}

    direction
minucone_normal(point p, void * cone__, bool at_second)
{
    direction d = cone_normal(p, cone__, at_second);
    scale(&d, -1);
    return d;
}

