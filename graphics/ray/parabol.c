//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "parabol.h"
#include <assert.h>


    static direction
transform_(point p, const parabol * parabol_)
{
    move(&p, parabol_->translate);
    direction endpoint = inverse_rotation(direction_from_origo(p), parabol_->rota);
    return endpoint;
}

    static void
transform_ray_(ray * ray_, const parabol * parabol_)
{
    move(&ray_->endpoint, parabol_->translate);
    inverse_rotation_ray(ray_, parabol_->rota);
}

    segment
parabol_intersection(
        const ray * ray_,
        const void * parabol__,
        int * hit)
{
    (void)hit;
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

    segment
_parabol_intersection(
        const ray * ray_,
        const void * parabol__,
        int * hit)
{
    return invert(parabol_intersection(ray_, parabol__, hit));
}

    direction
parabol_normal(point p, const void * parabol__, int hit)
{
    (void)hit;
    const parabol * parabol_ = parabol__;
    direction q = transform_(p, parabol_);
    direction n;
    if ((q.x == 0 && q.y == 0) || parabol_->r == 0) {
        n = (direction){0, 0, 1};
    } else {
        n = (direction){ -q.x, -q.y, 1 / parabol_->r };
        normalize(&n);
    }

    return rotation(n, parabol_->rota);
}

    direction
_parabol_normal(point p, const void * parabol__, int hit)
{
    direction d = parabol_normal(p, parabol__, hit);
    scale(&d, -1);
    return d;
}

