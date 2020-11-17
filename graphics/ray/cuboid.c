//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "cuboid.h"
#include "plane.h"

#define INTER_H_EXPORT_INTER
#include "inter.h"

#define MAKE_INTER(NAME, A, B) static segment NAME \
    (const ray * t, const void * arg, int * hit) { \
    (void)hit; (void)arg; \
    return axis_plane_intersection(A, B); }
MAKE_INTER(xn, 1 - t->endpoint.x, t->head.x)
MAKE_INTER(yn, 1 - t->endpoint.y, t->head.y)
MAKE_INTER(zn, 1 - t->endpoint.z, t->head.z)
MAKE_INTER(xm, 1 + t->endpoint.x, -t->head.x)
MAKE_INTER(ym, 1 + t->endpoint.y, -t->head.y)
MAKE_INTER(zm, 1 + t->endpoint.z, -t->head.z)

static inter_inter cuboid_inter = {
    6, {
    { xn, NULL, NULL }, { yn, NULL, NULL }, { zn, NULL, NULL },
    { xm, NULL, NULL }, { ym, NULL, NULL }, { zm, NULL, NULL }}
};

    static void
bases_ray(ray * ray_, direction s)
{
    ray_->head.x *= s.x;
    ray_->head.y *= s.y;
    ray_->head.z *= s.z;
    ray_->endpoint.x *= s.x;
    ray_->endpoint.y *= s.y;
    ray_->endpoint.z *= s.z;
}

    segment
cuboid_intersection(
        const ray * ray_,
        const void * cuboid__,
        int * hit)
{
    (void)hit;
    int dummy;
    ray t = *ray_;
    const cuboid * cuboid_ = cuboid__;
    move(&t.endpoint, cuboid_->translate);
    inverse_base_ray(&t, cuboid_->base);
    bases_ray(&t, cuboid_->s);

    return inter_intersection(&t, &cuboid_inter, &dummy);
}

segment
_cuboid_intersection(
        const ray * ray_,
        const void * cuboid__,
        int * hit)
{
    return invert(cuboid_intersection(ray_, cuboid__, hit));
}

    direction
cuboid_normal(point p, const void * cuboid__, int hit)
{
    (void)hit;
    const cuboid * cuboid_ = cuboid__;
    move(&p, cuboid_->translate);
    const direction m = inverse_base(direction_from_origo(p), cuboid_->base);
    const direction s = cuboid_->s;
    direction d = { rabs(m.x * s.x), rabs(m.y * s.y), rabs(m.z * s.z) };
    direction n = { 0, 0, 0 };
    if (d.x > d.y) {
        if (d.z > d.x) {
            n.z = m.z > 0 ? 1 : -1;
        } else  {
            n.x = m.x > 0 ? 1 : -1;
        }
    } else {
        if (d.z > d.y) {
            n.z = m.z > 0 ? 1 : -1;
        } else  {
            n.y = m.y > 0 ? 1 : -1;
        }
    }

    return base(n, cuboid_->base);
}

    direction
_cuboid_normal(point p, const void * cuboid__, int hit)
{
    direction d = cuboid_normal(p, cuboid__, hit);
    scale(&d, -1);
    return d;
}
