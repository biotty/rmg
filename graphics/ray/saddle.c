//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "saddle.h"
#include "matrix.h"


    static void
scale_ray(ray * ray_, real h)
{
    ray_->endpoint.z *= h;
    ray_->head.z *= h;
}

    static void
xy_ray(ray * ray_, const real R[4])
{
    const real m_endpoint[2] = { ray_->endpoint.x, ray_->endpoint.y };
    const real m_head[2] = { ray_->head.x, ray_->head.y };

    real r_endpoint[2];
    multiply(R, 2, 2, m_endpoint, r_endpoint);
    real r_head[2];
    multiply(R, 2, 2, m_head, r_head);

    ray_->endpoint.x = r_endpoint[0];
    ray_->endpoint.y = r_endpoint[1];
    ray_->head.x = r_head[0];
    ray_->head.y = r_head[1];
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
    inverse_rotation_ray(&t, saddle_->rota);
    const real v = - saddle_->v;
    const real R[4] = { cos(v), -sin(v), sin(v), cos(v) };
    xy_ray(&t, R);
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
    const direction m = inverse_rotation(direction_from_origo(p), saddle_->rota);
    const real m_xy[2] = { m.x, m.y };
    const real v = - saddle_->v;
    real R[4] = { cos(v), -sin(v), sin(v), cos(v) };
    real r_xy[2];
    multiply(R, 2, 2, m_xy, r_xy);
    r_xy[0] *= -2;
    r_xy[1] *= 2;
    R[1] *= -1;
    R[2] *= -1;
    real n_xy[2];
    multiply(R, 2, 2, r_xy, n_xy);
    direction n = { n_xy[0], n_xy[1], saddle_->h };
    normalize(&n);

    return rotation(n, saddle_->rota);
}
