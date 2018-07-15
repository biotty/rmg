//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "direction.h"
#include "matrix.h"

    direction
reflection(direction normal, direction d)
{
  /*assert(is_near(length(normal), 1));
    assert(is_near(length(d), 1));*/
    scale(&normal, -2 * scalar_product(normal, d));
    normal.x += d.x;
    normal.y += d.y;
    normal.z += d.z;
    //if ( ! is_near(length(normal), 1)) {
    //    normalize(&normal);
    //}
    return normal;
}

    direction
refraction(direction normal, direction d, real w)
{
  /*assert(is_near(length(normal), 1));
    assert(is_near(length(d), 1));*/
    scale(&d, w);
    const real a = - scalar_product(d, normal);
    const real det = 1 - square(w) * (1 - square(a));
    if (det < TINY_REAL) return DISORIENTED;
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
spherical_arg(direction d, real * r, rotation_arg * arg)
{
    real theta, phi;
    spherical(d, r, &theta, &phi);
    arg->theta_cos = cosf(theta);
    arg->theta_sin = sinf(theta);
    arg->phi_cos = cosf(phi);
    arg->phi_sin = sinf(phi);
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
transform(direction * d, const real trm[9])
{
    const real v[3] = {d->x, d->y, d->z};
    real r[3];
    multiply(trm, 3, 3, v, r);
    d->x = r[0];
    d->y = r[1];
    d->z = r[2];
}

    static inline void
rotate_xz_xy(direction  * d, const rotation_arg arg)
{
    const real ca = arg.theta_cos;
    const real sa = arg.theta_sin;
    const real cb = arg.phi_cos;
    const real sb = arg.phi_sin;
    const real RzRy[9] = {
        cb * ca, -sb, cb * sa,
        sb * ca, cb, sb * sa,
        -sa, 0, ca
    };

    transform(d, RzRy);
}

    static inline void
rotate_xy_xz(direction * d, const rotation_arg arg)
{
    const real ca = arg.theta_cos;
    const real sa = arg.theta_sin;
    const real cb = arg.phi_cos;
    const real sb = arg.phi_sin;
    const real RyRz[9] = {
        cb * ca, -cb * sa, sb,
        sa, ca, 0,
        -sb * ca, sb * sa, cb
    };

    transform(d, RyRz);
}

//
// rotation, inverse_rotation
//
// "tilt" would now be a better name as opposed to general axis-angle
// rotation but merely polar-tilts, and cannot represent i.e rotation
// on the z-axis.  this is why the the shapes by themselves define
// rotational disposition in adition to a (tilt) direction.
//
// a general rotation would use a full 3x3 rotation_arg
// (as opposed to current total of 4 values) but would
// not require two m.multiplications as of now, in addition
// to the expanding of the two matrices (xz and xy).
// the objects explicit rotation on its axis would go away.
//
// given these observations it seems that it would be beneficial
// for general time-performance to use the alternate approach
// of having a full axis-angle rotation-matrix, and profiling has
// been done identifying the rotation as an overall bottle-neck.
//
// note that with a general rotation matrix the
// inverse_rotation would trivially be to negate the angle.
//
// the presence of this comment means that converting the
// scene-objects (and decorations) to use the better approach
// has not yet been of priority, and the lack of renaming to
// tilt_ reflects continued desire to change this some day.
//
// however, it must be investigated whether a polar-tilt
// rotation-matrix has less than 9 different coefficients,
// as only the saddle would benefit from general rotation,
// all other shapes having symetry on z-axis.  then those
// coefficients replaces the ones currently in rotation_arg
// and rename to tilt.
//
// restricting to z-symetric shapes (except saddle) by choice
// for aesthetics and model simplicity -- to be considered
// -- and this may then impact the discussion in this comment
//
    direction
rotation(direction d, rotation_arg arg)
{
    rotate_xz_xy(&d, arg);
    return d;
}

    direction
inverse_rotation(direction d, rotation_arg arg)
{
    const rotation_arg inv = {
        arg.phi_cos,
        -arg.phi_sin,
        arg.theta_cos,
        -arg.theta_sin
    };

    rotate_xy_xz(&d, inv);
    return d;
}

    direction
norm_cross(direction a, direction b)
{
    direction ret = {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
    normalize(&ret);
    return ret;
}
