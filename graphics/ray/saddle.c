//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "saddle.h"
#include "matrix.h"


    static void
scale_ray(ray * ray_, const point * scale_)
{
    ray_->endpoint.x *= scale_->x;
    ray_->endpoint.y *= scale_->y;
    ray_->endpoint.z *= scale_->z;
    ray_->head.x *= scale_->x;
    ray_->head.y *= scale_->y;
    ray_->head.z *= scale_->z;
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

    real_pair
saddle_intersection(
        const ray * ray_,
        void * saddle__)
{
    ray t = *ray_;
    const saddle * saddle_ = saddle__;
    move(&t.endpoint, saddle_->translate);
    inverse_rotation_ray(&t, saddle_->theta, saddle_->phi);
    const real v = - saddle_->v;
    const real R[4] = { cos(v), -sin(v), sin(v), cos(v) };
    xy_ray(&t, R);
    scale_ray(&t, &saddle_->scale);

    const real a = square(t.head.x) - square(t.head.y);
    const real b = 2 * (t.endpoint.x * t.head.x - t.endpoint.y * t.head.y) - t.head.z;
    const real c = square(t.endpoint.x) - square(t.endpoint.y) - t.endpoint.z;
    return quadratic(a, b, c);
}

    direction
saddle_normal(point p, void * saddle__, bool at_second)
{
    (void)at_second;
    const saddle * saddle_ = saddle__;
    move(&p, saddle_->translate);
    const direction m = inverse_rotation(direction_from_origo(p),
            saddle_->theta, saddle_->phi);
    const real m_xy[2] = { m.x, m.y };
    const real v = - saddle_->v;
    real R[4] = { cos(v), -sin(v), sin(v), cos(v) };
    real r_xy[2];
    multiply(R, 2, 2, m_xy, r_xy);
    r_xy[0] *= -2 * saddle_->scale.x;
    r_xy[1] *= 2 * saddle_->scale.y;
    R[1] *= -1;
    R[2] *= -1;
    real n_xy[2];
    multiply(R, 2, 2, r_xy, n_xy);
    direction n = { n_xy[0], n_xy[1], saddle_->scale.z };
    normalize(&n);

    return rotation(n, saddle_->theta, saddle_->phi);
}
