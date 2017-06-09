//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "mapping.h"
#include "xmath.h"
#include "photo.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    photo * photo;
    texture_application a;
    real r;
    real theta;
    real phi;
} normal_decoration_arg;

typedef struct {
    photo * photo;
    texture_application a;
    real r;
    real theta;
    real phi;
} planar_decoration_arg;

typedef struct {
    photo * photo;
    texture_application a;
    point o;
    real r;
    real theta;
    real phi;
} relative_decoration_arg;

typedef struct {
    photo * photo;
    texture_application a;
    point o;
    real r;
    real theta;
    real phi;
} axial_decoration_arg;

    void
delete_texture_mapping(void * decoration_arg)
{
    normal_decoration_arg * da = decoration_arg;
    photo_delete(da->photo);
    free(da);
}

    static compact_color
linear(compact_color x, color a, compact_color b)
{
    compact_color ret = {
        x.r * a.r + b.r,
        x.g * a.g + b.g,
        x.b * a.b + b.b
    };
    return ret;
}

    static void
texture_map(const texture_application * a, const photo * ph, real x, real y,
        object_optics * so, const object_optics * adjust)
{
    so->refraction_index_nano = adjust->refraction_index_nano;
    so->passthrough_filter = adjust->passthrough_filter;
    if ( ! ph) {
        so->reflection_filter = adjust->reflection_filter;
        so->absorption_filter = adjust->absorption_filter;
        so->refraction_filter = adjust->refraction_filter;
        return;
    }
    const compact_color c = photo_color(ph, x * ph->width, y * ph->height);
    so->reflection_filter = linear(
            c, a->reflection_factor, adjust->reflection_filter);
    so->absorption_filter = linear(
            c, a->absorption_factor, adjust->absorption_filter);
    so->refraction_filter = linear(
            c, a->refraction_factor, adjust->refraction_filter);
}

    static void
zoom_(const real r, real * x_, real * y_)
{
    *x_ = zoom(r, *x_);
    *y_ = zoom(r, *y_);
}

    static void
wrap_(const texture_application * a, real * x, real * y)
{
    *x += a->x_wrap;
    if (*x >= 1) *x -= 1;
    *y += a->y_wrap;
    if (*y >= 1) *y -= 1;
}

    static void
normal_decoration(const ray * ray_, void * decoration_arg,
        object_optics * so, const object_optics * adjust)
{
    const normal_decoration_arg * da = decoration_arg;
    real x, y;
    direction d = inverse_rotation(ray_->head, da->theta, da->phi);
    direction_to_unitsquare(&d, &x, &y);
    zoom_(da->r, &x, &y);
    wrap_(&da->a, &x, &y);
    texture_map(&da->a, da->photo, x, y, so, adjust);
}

    static void
planar_decoration(const ray * ray_, void * decoration_arg,
        object_optics * so, const object_optics * adjust)
{
    const planar_decoration_arg * da = decoration_arg;
    direction d = inverse_rotation(
            direction_from_origo(ray_->endpoint),
            da->theta, da->phi);
    real x = d.x;
    real y = d.y;
    zoom_(da->r, &x, &y);
    wrap_(&da->a, &x, &y);
    texture_map(&da->a, da->photo, x, y, so, adjust);
}

    static void
relative_decoration(const ray * ray_, void * decoration_arg,
        object_optics * so, const object_optics * adjust)
{
    const relative_decoration_arg * da = decoration_arg;
    direction d = inverse_rotation(
            distance_vector(da->o, ray_->endpoint),
            da->theta, da->phi);
    real x, y;
    direction_to_unitsquare(&d, &x, &y);
    zoom_(da->r, &x, &y);
    wrap_(&da->a, &x, &y);
    texture_map(&da->a, da->photo, x, y, so, adjust);
}

    static void
axial_decoration(const ray * ray_, void * decoration_arg,
        object_optics * so, const object_optics * adjust)
{
    static const real pi = REAL_PI;
    static const real two_pi = REAL_PI * 2;
    const axial_decoration_arg * da = decoration_arg;
    direction d = inverse_rotation(
            distance_vector(da->o, ray_->endpoint),
            da->theta, da->phi);
    real x = (ratan(d.y, d.x) + pi) / two_pi;
    real y = d.z * da->r;
    y -= rfloor(y);
    if (y == 1) y = 0;
    wrap_(&da->a, &x, &y);
    texture_map(&da->a, da->photo, x, y, so, adjust);
}

    void *
normal_texture_mapping(object_decoration * df, direction n,
        const char * path, texture_application a)
{
    normal_decoration_arg * da = malloc(sizeof *da);
    da->a = a;
    da->photo = photo_create(path);
    real r;
    spherical(n, &r, &da->theta, &da->phi);
    da->r = 1 / r;
    *df = normal_decoration;
    return da;
}

    void *
planar_texture_mapping(object_decoration * df, direction n,
        const char * path, texture_application a)

{
    planar_decoration_arg * da = malloc(sizeof *da);
    da->a = a;
    da->photo = photo_create(path);
    real r;
    spherical(n, &r, &da->theta, &da->phi);
    da->r = 1 / r;
    *df = planar_decoration;
    return da;
}

    void *
relative_texture_mapping(object_decoration * df, direction n,
        point o, const char * path, texture_application a)
{
    relative_decoration_arg * da = malloc(sizeof *da);
    da->a = a;
    da->photo = photo_create(path);
    real r;
    spherical(n, &r, &da->theta, &da->phi);
    da->r = 1 / r;
    da->o = o;
    *df = relative_decoration;
    return da;
}

    void *
axial_texture_mapping(object_decoration * df, direction n,
        point o, const char * path, texture_application a)
{
    axial_decoration_arg * da = malloc(sizeof *da);
    da->a = a;
    da->photo = photo_create(path);
    real r;
    spherical(n, &r, &da->theta, &da->phi);
    da->r = 1 / r;
    da->o = o;
    *df = axial_decoration;
    return da;
}
