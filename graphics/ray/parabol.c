//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "parabol.h"
#include <assert.h>


    static direction
transform_(point p, const parabol * parabol_)
{
    move(&p, parabol_->translate);
    direction endpoint = inverse_rotation(direction_from_origo(p), parabol_->theta, parabol_->phi);
    return endpoint;
}

    static void
transform_ray_(ray * ray_, const parabol * parabol_)
{
    move(&ray_->endpoint, parabol_->translate);
    inverse_rotation_ray(ray_, parabol_->theta, parabol_->phi);
}

    real_pair
parabol_intersection(
        const ray * ray_,
        void * parabol__)
{
    ray t = *ray_;
    const parabol * parabol_ = parabol__;
    transform_ray_(&t, parabol_);

    const real a = square(t.head.x) + square(t.head.y);
    const real b = 2 * ( - parabol_->r * t.head.z
        + t.endpoint.x * t.head.x
        + t.endpoint.y * t.head.y);
    const real c = square(t.endpoint.x) + square(t.endpoint.y) + square(parabol_->r)
        - 2 * parabol_->r * t.endpoint.z;
    return quadratic(a, b, c);
}

    real_pair
_parabol_intersection(
        const ray * ray_,
        void * parabol__)
{
    return invert(_parabol_intersection(ray_, parabol__));
}

    direction
parabol_normal(point p, void * parabol__, bool at_second)
{
    (void)at_second;
    const parabol * parabol_ = parabol__;
    direction q = transform_(p, parabol_);
    direction n;
    if ((q.x == 0 && q.y == 0) || parabol_->r == 0) {
        n = (direction){0, 0, 1};
    } else {
        n = (direction){ -q.x, -q.y, 1 / parabol_->r };
        normalize(&n);
    }

    return rotation(n, parabol_->theta, parabol_->phi);
}

    direction
_parabol_normal(point p, void * parabol__, bool at_second)
{
    direction d = _parabol_normal(p, parabol__, at_second);
    scale(&d, -1);
    return d;
}

