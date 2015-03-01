//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "sphere.h"
#include "xmath.h"
#include <math.h>
#include <assert.h>

real_pair
sphere_intersection(
        const ray * ray_,
        void * sphere__)
{
    const sphere * sphere_ = sphere__;
    const direction d = distance_vector(sphere_->center, ray_->endpoint);
    const real b = 2 * scalar_product(d, ray_->head);
    const real c = square(d.x) + square(d.y) + square(d.z)
                   - square(sphere_->radius);
    const real det = square(b) - 4 * c;
    if (det <= 0) return (real_pair){-1, -1};
    const real sqrt_= sqroot(det);
    return (real_pair){
        0.5 * (-b - sqrt_),
        0.5 * (-b + sqrt_)
    };
}

real_pair
minusphere_intersection(
        const ray * ray_,
        void * sphere__)
{
    real_pair p = sphere_intersection(ray_, sphere__);
    if (p.first >= 0) {
        assert(p.second >= 0);
        return (real_pair){p.second, p.first};
    } else
        return (real_pair){p.second, HUGE_REAL};
}

    direction
sphere_normal(point p, void * sphere__, bool at_second)
{
    (void)at_second;
    const sphere * sphere_ = sphere__;
    p.x -= sphere_->center.x;
    p.y -= sphere_->center.y;
    p.z -= sphere_->center.z;
    direction d = direction_from_origo(p);
    normalize(&d);
    return d;
}

    direction
minusphere_normal(point p, void * sphere__, bool at_second)
{
    direction d = sphere_normal(p, sphere__, at_second);
    scale(&d, -1);
    return d;
}

