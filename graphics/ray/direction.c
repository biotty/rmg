//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "direction.h"
#include "matrix.h"
#include "xmath.h"

#ifndef point_from_origo
    point
point_from_origo(direction direction_)
{
    return (point){direction_.x, direction_.y, direction_.z};
}
#endif
#ifndef direction_from_origo
    direction
direction_from_origo(point point_)
{
    return (direction){point_.x, point_.y, point_.z};
}
#endif

    real
length(direction direction_)
{
    return distance_to_origo(point_from_origo(direction_));
}

    void
scale(direction * direction_, real r)
{
    direction_->x *= r;
    direction_->y *= r;
    direction_->z *= r;
}

    void
normalize(direction * direction_)
{
    scale(direction_, 1 / length(*direction_));
}

    real
scalar_product(direction a, direction b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

    direction
reflection(direction normal, direction d)
{
  /*assert(is_near(length(normal), 1));
    assert(is_near(length(d), 1));*/
    scale(&normal, -2 * scalar_product(normal, d));
    normal.x += d.x;
    normal.y += d.y;
    normal.z += d.z;
    if ( ! is_near(length(normal), 1)) {
        normalize(&normal);
    }
    return normal;
}

    void
move(point * p, direction displacement)
{
    p->x += displacement.x;
    p->y += displacement.y;
    p->z += displacement.z;
}

    direction
distance_vector(point from, point to)
{
    return (direction){
        to.x - from.x,
            to.y - from.y,
            to.z - from.z
    };
}

    direction
refraction(direction normal, direction d, real w, real minimum_det)
{
  /*assert(is_near(length(normal), 1));
    assert(is_near(length(d), 1));*/
    scale(&d, w);
    const real a = - scalar_product(d, normal);
    const real det = 1 - square(w) * (1 - square(a));
    if (det < minimum_det) return DISORIENTED;
    const real b = sqroot(det);
    scale(&normal, w * a - b);
    d.x += normal.x;
    d.y += normal.y;
    d.z += normal.z;
    normalize(&d);
    return d;
}

    void
spherical(direction d, real * r, real * theta, real * phi)
{
  /*assert(!isnan(d.x));
    assert(!isnan(d.y));
    assert(!isnan(d.z));*/
    const real r_ = length(d);
    const real S = sqroot(square(d.x) + square(d.y));
    if (S == 0) {
        *theta = d.z > 0 ? 0 : REAL_PI;
        *phi = 0/*whatever*/;
    } else {
        const real theta_ = acos(d.z / r_);
        const real phi_ = (d.x >= 0)
            ? asin(d.y / S)
            : REAL_PI - asin(d.y / S);
        *theta = theta_;
        *phi = isnan(phi_) ? 0 : phi_; //hmm, why do we see -NaN
    }
    *r = r_;
}

    void
direction_to_unitsquare(const direction * d, real * x, real * y)
{
    static const real pi = REAL_PI;
    static const real pi_2 = REAL_PI / 2;
    static const real two_pi = REAL_PI * 2;
    real r_ignored, theta, phi;
    spherical(*d, &r_ignored, &theta, &phi);
    *x = (phi + pi_2) / two_pi;
    *y = theta / pi;
}   

    static void
transform(direction * d, const real T[9])
{
    const real v[3] = {d->x, d->y, d->z};
    real r[3];
    multiply(T, 3, 3, v, r);
    d->x = r[0];
    d->y = r[1];
    d->z = r[2];
}

    static void
rotate_xy(direction  * d, real a)
{
    const real Rz[3 * 3] = {
        cos(a), -sin(a), 0,
        sin(a), cos(a), 0,
        0, 0, 1,
    };
    transform(d, Rz);
}

    static void
rotate_xz(direction * d, real a)
{
    const real Ry[3 * 3] = {
        cos(a), 0, sin(a),
        0, 1, 0,
        -sin(a), 0, cos(a),
    };
    transform(d, Ry);
}

#if 0 /*unused*/
    static void
rotate_yz(direction * d, real a)
{
    const real Rx[3 * 3] = {
        1, 0, 0,
        0, cos(a), -sin(a),
        0, sin(a), cos(a),
    };
    transform(d, Rx);
}
#endif

    direction
rotation(direction d, real theta, real phi)
{
    rotate_xz(&d, theta);
    rotate_xy(&d, phi);
    return d;
}

    direction
inverse_rotation(direction d, real theta, real phi)
{
    rotate_xy(&d, - phi);
    rotate_xz(&d, - theta);
    return d;
}

