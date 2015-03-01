//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "map.h"
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
} positional_decoration_arg;

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
} linear_decoration_arg;

    void
delete_texture_mapping(void * decoration_arg)
{
    positional_decoration_arg * da = decoration_arg;
    photo_delete(da->photo);
    free(da);
}

    static color
linear(color x, color a, color b)
{
    filter(&x, a);
    return optical_sum(x, b);
}

    static void
texture_map(const texture_application * a, const photo * ph, real x, real y,
        object_optics * so, const object_optics * adjust)
{
    so->refraction_index = adjust->refraction_index;
    so->passthrough_filter = adjust->passthrough_filter;
    if ( ! ph) {
        so->reflection_filter = adjust->reflection_filter;
        so->absorption_filter = adjust->absorption_filter;
        so->refraction_filter = adjust->refraction_filter;
        return;
    }
    const color c = photo_color(ph, x * ph->width, y * ph->height);
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
directional_decoration(const ray * ray_, void * decoration_arg,
        object_optics * so, const object_optics * adjust)
{
    const positional_decoration_arg * da = decoration_arg;
    real x, y;
    direction d = inverse_rotation(ray_->head, da->theta, da->phi);
    direction_to_unitsquare(&d, &x, &y);
    zoom_(da->r, &x, &y);
    wrap_(&da->a, &x, &y);
    texture_map(&da->a, da->photo, x, y, so, adjust);
}

    static void
positional_decoration(const ray * ray_, void * decoration_arg,
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
linear_decoration(const ray * ray_, void * decoration_arg,
        object_optics * so, const object_optics * adjust)
{
    static const real pi = REAL_PI;
    static const real two_pi = REAL_PI * 2;
    const linear_decoration_arg * da = decoration_arg;
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
directional_texture_mapping(object_decoration * df, direction n,
        const char * path, texture_application a)
{
    positional_decoration_arg * da = malloc(sizeof *da);
    da->a = a;
    da->photo = photo_create(path);
    real r;
    spherical(n, &r, &da->theta, &da->phi);
    da->r = 1 / r;
    *df = directional_decoration;
    return da;
}

    void *
positional_texture_mapping(object_decoration * df, direction n,
        const char * path, texture_application a)

{
    planar_decoration_arg * da = malloc(sizeof *da);
    da->a = a;
    da->photo = photo_create(path);
    real r;
    spherical(n, &r, &da->theta, &da->phi);
    da->r = 1 / r;
    *df = positional_decoration;
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
linear_texture_mapping(object_decoration * df, direction n,
        point o, const char * path, texture_application a)
{
    linear_decoration_arg * da = malloc(sizeof *da);
    da->a = a;
    da->photo = photo_create(path);
    real r;
    spherical(n, &r, &da->theta, &da->phi);
    da->r = 1 / r;
    da->o = o;
    *df = linear_decoration;
    return da;
}
