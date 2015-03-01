//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "plane.h"
#include "xmath.h"

real_pair
plane_intersection(
        const ray * ray_,
        void * plane__)
{
    const plane * plane_ = plane__;
    const real b = scalar_product(plane_->normal, ray_->head);
    const direction v = distance_vector(ray_->endpoint, plane_->at_surface);
    const real a = scalar_product(plane_->normal, v);
    if (is_near(b, 0)) {
        return (real_pair){-1, (a >= 0) ? HUGE_REAL : -1};
    }
    if (b < 0 && a >= 0) return (real_pair){-1, HUGE_REAL};
    if (b > 0 && a <= 0) return (real_pair){-1, -1};
    const real r = a / b;
    if (b < 0)
        return (real_pair){r, HUGE_REAL};
    else
        return (real_pair){-1, r};
}

direction
plane_normal(point p, void * plane__, bool at_second)
{
    (void)at_second;
    const plane * plane_ = plane__;
    const direction d = plane_->normal;
    (void)p;
    return d;
}

