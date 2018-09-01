//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "mapping.h"
#include "photo.h"
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    photo * photo;
    texture_application a;
    real r;
    tilt_arg rota;
    real cos_w;
    real sin_w;
} texture_arg;

typedef struct {
    photo * photo;
    texture_application a;
    point o;
    real r;
    tilt_arg rota;
    real cos_w;
    real sin_w;
} texture_origin_arg;

typedef struct {
    photo * photo;
    texture_application a;
    point o;
    real r;
    tilt_arg rota;
    real w;
} texture_axial_arg;

    void
delete_decoration(void * decoration_arg)
{
    texture_arg * da = decoration_arg;
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
texture_map(const texture_application * t, const photo * ph, real x, real y,
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
    const photo_attr * a = (photo_attr *) ph;
    const compact_color c = photo_color(ph, x * a->width, y * a->height);
    so->reflection_filter = linear(
            c, t->reflection_factor, adjust->reflection_filter);
    so->absorption_filter = linear(
            c, t->absorption_factor, adjust->absorption_filter);
    so->refraction_filter = linear(
            c, t->refraction_factor, adjust->refraction_filter);
}

    static void
zoom_(const real r, real * x, real * y, bool repeat)
{
    *x = *x * r + .5;
    *y = *y * r + .5;
    if (repeat) {
        mod1(x);
        mod1(y);
    } else {
        if (*x < 0 || *x >= 1 || *y < 0 || *y >= 1)
            *x = *y = 0;
    }
}

    static void
rotate_(const real cos_w, const real sin_w, real * x_, real * y_)
{
    const real x
        = *x_ * cos_w - *y_ * sin_w;
    *y_ = *x_ * sin_w + *y_ * cos_w;
    *x_ = x;
}

    static void
normal_decoration(const ray * ray_, const void * decoration_arg,
        object_optics * so, const object_optics * adjust)
{
    const texture_arg * da = decoration_arg;
    real x, y;
    direction d = inverse_tilt(ray_->head, da->rota);
    rotate_(da->cos_w, da->sin_w, &d.x, &d.y);
    direction_to_unitsquare(&d, &x, &y);
    zoom_(da->r, &x, &y, false);
    texture_map(&da->a, da->photo, x, y, so, adjust);
}

    static void
planar_decoration_(const ray * ray_, const void * decoration_arg,
        object_optics * so, const object_optics * adjust, bool repeat)
{
    const texture_origin_arg * da = decoration_arg;
    direction d = inverse_tilt(
            distance_vector(da->o, ray_->endpoint), da->rota);
    real x = d.x;
    real y = d.y;
    rotate_(da->cos_w, da->sin_w, &x, &y);
    zoom_(da->r, &x, &y, repeat);
    texture_map(&da->a, da->photo, x, y, so, adjust);
}

    static void
planar_decoration(const ray * ray_, const void * decoration_arg,
        object_optics * so, const object_optics * adjust)
{
    planar_decoration_(ray_, decoration_arg, so, adjust, true);
}

    static void
planar1_decoration(const ray * ray_, const void * decoration_arg,
        object_optics * so, const object_optics * adjust)
{
    planar_decoration_(ray_, decoration_arg, so, adjust, false);
}

    static void
relative_decoration(const ray * ray_, const void * decoration_arg,
        object_optics * so, const object_optics * adjust)
{
    const texture_origin_arg * da = decoration_arg;
    real x, y;
    direction d = inverse_tilt(
            distance_vector(da->o, ray_->endpoint), da->rota);
    rotate_(da->cos_w, da->sin_w, &d.x, &d.y);
    direction_to_unitsquare(&d, &x, &y);
    zoom_(da->r, &x, &y, false);
    texture_map(&da->a, da->photo, x, y, so, adjust);
}

    static void
axial_decoration_(const ray * ray_, const void * decoration_arg,
        object_optics * so, const object_optics * adjust, bool repeat)
{
    static const real pi = REAL_PI;
    static const real two_pi = REAL_PI * 2;
    const texture_axial_arg * da = decoration_arg;
    direction d = inverse_tilt(
            distance_vector(da->o, ray_->endpoint), da->rota);
    real x = 1;
    real y = d.z * da->r;
    if (repeat) {
        y -= rfloor(y);
        if (y == 1)
            y = 0;
    } else {
        y += .5;
        if (y < 0 || y >= 1) {
            y = 0;
            x = 0;
        }
    }
    if (x == 1) {
        x = (ratan(d.y, d.x) + pi + da->w) / two_pi;
        x -= rfloor(x);
    }
    texture_map(&da->a, da->photo, x, y, so, adjust);
}

    static void
axial_decoration(const ray * ray_, const void * decoration_arg,
        object_optics * so, const object_optics * adjust)
{
    axial_decoration_(ray_, decoration_arg, so, adjust, true);
}

    static void
axial1_decoration(const ray * ray_, const void * decoration_arg,
        object_optics * so, const object_optics * adjust)
{
    axial_decoration_(ray_, decoration_arg, so, adjust, false);
}

    void *
normal_texture_mapping(object_decoration * df,
        tilt_arg rota, real r, real w,
        const char * path, texture_application a)
{
    texture_arg * da = malloc(sizeof *da);
    da->a = a;
    da->photo = photo_create(path);
    da->rota = rota;
    da->r = 1 / r;
    da->cos_w = cos(-w);
    da->sin_w = sin(-w);
    *df = normal_decoration;
    return da;
}

    static void *
_planar_texture_mapping(tilt_arg rota, real r, real w, point o,
        const char * path, texture_application a)

{
    texture_origin_arg * da = malloc(sizeof *da);
    da->a = a;
    da->photo = photo_create(path);
    da->rota = rota;
    da->r = 1 / r;
    da->cos_w = cos(-w);
    da->sin_w = sin(-w);
    da->o = o;
    return da;
}

    void *
planar_texture_mapping(object_decoration * df,
        tilt_arg rota, real r, real w, point o,
        const char * path, texture_application a)

{
    *df = planar_decoration;
    return _planar_texture_mapping(rota, r, w, o, path, a);
}

    void *
planar1_texture_mapping(object_decoration * df,
        tilt_arg rota, real r, real w, point o,
        const char * path, texture_application a)

{
    *df = planar1_decoration;
    return _planar_texture_mapping(rota, r, w, o, path, a);
}

    void *
relative_texture_mapping(object_decoration * df,
        tilt_arg rota, real r, real w, point o,
        const char * path, texture_application a)
{
    texture_origin_arg * da = malloc(sizeof *da);
    da->a = a;
    da->photo = photo_create(path);
    da->rota = rota;
    da->r = 1 / r;
    da->cos_w = cos(-w);
    da->sin_w = sin(-w);
    da->o = o;
    *df = relative_decoration;
    return da;
}

    static void *
_axial_texture_mapping(tilt_arg rota, real r, real w, point o,
        const char * path, texture_application a)
{
    texture_axial_arg * da = malloc(sizeof *da);
    da->a = a;
    da->photo = photo_create(path);
    da->rota = rota;
    da->r = 1 / r;
    da->w = -w;
    da->o = o;
    return da;
}

    void *
axial_texture_mapping(object_decoration * df,
        tilt_arg rota, real r, real w, point o,
        const char * path, texture_application a)
{
    *df = axial_decoration;
    return _axial_texture_mapping(rota, r, w, o, path, a);
}

    void *
axial1_texture_mapping(object_decoration * df,
        tilt_arg rota, real r, real w, point o,
        const char * path, texture_application a)
{
    *df = axial1_decoration;
    return _axial_texture_mapping(rota, r, w, o, path, a);
}
