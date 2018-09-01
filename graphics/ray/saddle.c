//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "saddle.h"


    static void
scale_ray(ray * ray_, real h)
{
    ray_->endpoint.z *= h;
    ray_->head.z *= h;
}

    segment
saddle_intersection(
        const ray * ray_,
        const void * saddle__,
        int * hit)
{
    (void)hit;
    ray t = *ray_;
    const saddle * saddle_ = saddle__;
    move(&t.endpoint, saddle_->translate);
    inverse_base_ray(&t, saddle_->base);
    scale_ray(&t, saddle_->h);

    const real a = square(t.head.x) - square(t.head.y);
    const real b = 2 * (t.endpoint.x * t.head.x - t.endpoint.y * t.head.y) - t.head.z;
    const real c = square(t.endpoint.x) - square(t.endpoint.y) - t.endpoint.z;
    return quadratic(a, b, c);
}

    direction
saddle_normal(point p, const void * saddle__, int hit)
{
    (void)hit;
    const saddle * saddle_ = saddle__;
    move(&p, saddle_->translate);
    const direction m = inverse_base(direction_from_origo(p), saddle_->base);
    direction n = { m.x * -2, m.y * 2, saddle_->h };
    normalize(&n);

    return base(n, saddle_->base);
}
