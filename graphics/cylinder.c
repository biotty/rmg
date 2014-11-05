//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "cylinder.h"
#include "math.h"
#include <math.h>
#include <stdio.h>
#include <assert.h>


    static direction
transform_(point p, const cylinder * cylinder_)
{
    move(&p, cylinder_->translate);
    return inverse_rotation(direction_from_origo(p), cylinder_->theta, cylinder_->phi);
}

    pair
cylinder_intersection(
        const ray * ray_,
        void * cylinder__)
{
    const cylinder * cylinder_ = cylinder__;
    const direction endpoint = transform_(ray_->endpoint, cylinder_);
    const direction head = inverse_rotation(ray_->head, cylinder_->theta, cylinder_->phi);

    const real h_x = head.x;
    const real h_y = head.y;
    const real e_x = endpoint.x;
    const real e_y = endpoint.y;
    const real sq_r = cylinder_->r;
    const real sq_e = square(e_x) + square(e_y);
    const real sq_h = square(h_x) + square(h_y);
    if (is_near(sq_h, 0)) {
        if (sq_e > sq_r) 
            return (pair){-1, -1};
        else
            return (pair){-1, HUGE_REAL};
    }
    const real a = sq_h;
    const real b = 2 * (e_x * h_x + e_y * h_y);
    const real c = sq_e - sq_r;
    
    const real det = square(b) - 4 * a * c;
    if (det <= 0) {
        return (pair){-1, -1};
    }
    const real sqrt_det = sqroot(det);
    
    const real f = 1 / (2 * a);
    assert(f >= 0);
    const real t1 = f * (-b - sqrt_det);
    const real t2 = f * (-b + sqrt_det);
    return (pair){t1, t2};
}

pair
minucylinder_intersection(
        const ray * ray_,
        void * cylinder__)
{
    pair p = cylinder_intersection(ray_, cylinder__);
    if (p.first >= 0) {
        assert(p.second >= 0);
        return (pair){p.second, p.first};
    } else
        return (pair){p.second, HUGE_REAL};
}

    direction
cylinder_normal(point p, void * cylinder__, bool at_second)
{
    const cylinder * cylinder_ = cylinder__;
    direction n = transform_(p, cylinder_);
    
    n.z = 0;
    normalize(&n);

    return rotation(n, cylinder_->theta, cylinder_->phi);
}

    direction
minucylinder_normal(point p, void * cylinder__, bool at_second)
{
    direction d = cylinder_normal(p, cylinder__, at_second);
    scale(&d, -1);
    return d;
}

