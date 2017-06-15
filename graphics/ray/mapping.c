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
} texture_arg;

typedef struct {
    photo * photo;
    texture_application a;
    point o;
    real r;
    real theta;
    real phi;
} texture_origin_arg;

typedef struct {
    intptr_t q; // value: as provisioned -- frag. of data q
    // ^ tag: small, determinant distinguished from photo *
    compact_color reflection_filter;
    compact_color absorption_filter;
    compact_color refraction_filter;
    point o;
    real r;
    real theta;
    real phi;
    char data[];
} checkers_arg;

#define NONPTR_MAX 511

    void
delete_decoration(void * decoration_arg)
{
    texture_arg * da = decoration_arg;
    const intptr_t q = (intptr_t)da->photo;
    if (q > NONPTR_MAX) photo_delete(da->photo);
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

    static compact_color
mix(compact_color a, compact_color b, real s)
{
    const real t = 1 - s;
    compact_color ret = {
        a.r * s + b.r * t,
        a.g * s + b.g * t,
        a.b * s + b.b * t
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
zoom_(const real r, real * x, real * y)
{
    *x = zoom(r, *x);
    *y = zoom(r, *y);
}

    static void
normal_decoration(const ray * ray_, void * decoration_arg,
        object_optics * so, const object_optics * adjust)
{
    const texture_arg * da = decoration_arg;
    real x, y;
    direction d = inverse_rotation(ray_->head, da->theta, da->phi);
    direction_to_unitsquare(&d, &x, &y);
    zoom_(da->r, &x, &y);
    texture_map(&da->a, da->photo, x, y, so, adjust);
}

    static void
planar_decoration_(const ray * ray_, void * decoration_arg,
        object_optics * so, const object_optics * adjust, bool repeat)
{
    const texture_origin_arg * da = decoration_arg;
    direction d = inverse_rotation(
            distance_vector(da->o, ray_->endpoint),
            da->theta, da->phi);
    real x = d.x;
    real y = d.y;
    if (repeat) zoom_(da->r, &x, &y);
    else {
        x = .5 + x * da->r;
        y = .5 + y * da->r;
        if (x < 0 || x >= 1 || y < 0 || y >= 1)
            x = y = 0;
    }
    texture_map(&da->a, da->photo, x, y, so, adjust);
}

    static void
planar_decoration(const ray * ray_, void * decoration_arg,
        object_optics * so, const object_optics * adjust)
{
    planar_decoration_(ray_, decoration_arg, so, adjust, true);
}

    static void
planar1_decoration(const ray * ray_, void * decoration_arg,
        object_optics * so, const object_optics * adjust)
{
    planar_decoration_(ray_, decoration_arg, so, adjust, false);
}

    static void
relative_decoration(const ray * ray_, void * decoration_arg,
        object_optics * so, const object_optics * adjust)
{
    const texture_origin_arg * da = decoration_arg;
    direction d = inverse_rotation(
            distance_vector(da->o, ray_->endpoint),
            da->theta, da->phi);
    real x, y;
    direction_to_unitsquare(&d, &x, &y);
    zoom_(da->r, &x, &y);
    texture_map(&da->a, da->photo, x, y, so, adjust);
}

    static void
axial_decoration_(const ray * ray_, void * decoration_arg,
        object_optics * so, const object_optics * adjust, bool repeat)
{
    static const real pi = REAL_PI;
    static const real two_pi = REAL_PI * 2;
    const texture_origin_arg * da = decoration_arg;
    direction d = inverse_rotation(
            distance_vector(da->o, ray_->endpoint),
            da->theta, da->phi);
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
    if (x == 1)
        x = (ratan(d.y, d.x) + pi) / two_pi;
    texture_map(&da->a, da->photo, x, y, so, adjust);
}

    static void
axial_decoration(const ray * ray_, void * decoration_arg,
        object_optics * so, const object_optics * adjust)
{
    axial_decoration_(ray_, decoration_arg, so, adjust, true);
}

    static void
axial1_decoration(const ray * ray_, void * decoration_arg,
        object_optics * so, const object_optics * adjust)
{
    axial_decoration_(ray_, decoration_arg, so, adjust, false);
}

    static void
checkers_decoration(const ray * ray_, void * decoration_arg,
        object_optics * so, const object_optics * b)
{
    const checkers_arg * da = decoration_arg;
    direction d = inverse_rotation(
            distance_vector(da->o, ray_->endpoint),
            da->theta, da->phi);
    scale(&d, da->r);
    const int q = 4 * da->q;
    const int xi = (d.x - rfloor(d.x)) * q;
    const int yi = (d.y - rfloor(d.y)) * q;
    const int zi = (d.z - rfloor(d.z)) * q;
    const unsigned char xv = da->data[xi];
    const unsigned char yv = da->data[q + yi];
    const unsigned char zv = da->data[q * 2 + zi];

    so->refraction_index_nano = b->refraction_index_nano;
    so->passthrough_filter = b->passthrough_filter;
    const real v = (xv ^ yv ^ zv) / 255.0;
    so->reflection_filter = mix(da->reflection_filter,
            b->reflection_filter, v);
    so->absorption_filter = mix(da->absorption_filter,
            b->absorption_filter, v);
    so->refraction_filter = mix(da->refraction_filter,
            b->refraction_filter, v);
}

    void *
normal_texture_mapping(object_decoration * df, direction n,
        const char * path, texture_application a)
{
    texture_arg * da = malloc(sizeof *da);
    da->a = a;
    da->photo = photo_create(path);
    real r;
    spherical(n, &r, &da->theta, &da->phi);
    da->r = 1 / r;
    *df = normal_decoration;
    return da;
}

    static void *
_planar_texture_mapping(direction n,
        point o, const char * path, texture_application a)

{
    texture_origin_arg * da = malloc(sizeof *da);
    da->a = a;
    da->photo = photo_create(path);
    real r;
    spherical(n, &r, &da->theta, &da->phi);
    da->r = 1 / r;
    da->o = o;
    return da;
}

    void *
planar_texture_mapping(object_decoration * df, direction n,
        point o, const char * path, texture_application a)

{
    *df = planar_decoration;
    return _planar_texture_mapping(n, o, path, a);
}

    void *
planar1_texture_mapping(object_decoration * df, direction n,
        point o, const char * path, texture_application a)

{
    *df = planar1_decoration;
    return _planar_texture_mapping(n, o, path, a);
}

    void *
relative_texture_mapping(object_decoration * df, direction n,
        point o, const char * path, texture_application a)
{
    texture_origin_arg * da = malloc(sizeof *da);
    da->a = a;
    da->photo = photo_create(path);
    real r;
    spherical(n, &r, &da->theta, &da->phi);
    da->r = 1 / r;
    da->o = o;
    *df = relative_decoration;
    return da;
}

    static void *
_axial_texture_mapping(direction n,
        point o, const char * path, texture_application a)
{
    texture_origin_arg * da = malloc(sizeof *da);
    da->a = a;
    da->photo = photo_create(path);
    real r;
    spherical(n, &r, &da->theta, &da->phi);
    da->r = 1 / r;
    da->o = o;
    return da;
}

    void *
axial_texture_mapping(object_decoration * df, direction n,
        point o, const char * path, texture_application a)
{
    *df = axial_decoration;
    return _axial_texture_mapping(n, o, path, a);
}

    void *
axial1_texture_mapping(object_decoration * df, direction n,
        point o, const char * path, texture_application a)
{
    *df = axial1_decoration;
    return _axial_texture_mapping(n, o, path, a);
}

    typedef short beam_t;
    static int cmp(const void * a, const void * b)
{
    return *(beam_t *)a - *(beam_t *)b;
}

    static void
beams(char * a, int a_size, int n)
{
    beam_t * b = malloc(n * sizeof *b);
    for (int i=0; i<n; i++) {
        const int r = rand();
        b[i] = (0xffff & (r ^ (r >> 16))) * a_size / 0xffff;
    }
    qsort(b, n, sizeof (beam_t), cmp);
    for (int j=0, i=0; i<n; i++) {
        const int k = b[i];
        const int r = rand();
        unsigned char v = r ^ (r >> 16);
        if (i & 1) v &= 127;
        else v |= 128;
        while (j < k) a[j++] = v;
    }
    free(b);
}

    void *
checkers_mapping(object_decoration * df, direction n, point o, int w,
        compact_color reflection_filter,
        compact_color absorption_filter,
        compact_color refraction_filter)
{
    if (w > NONPTR_MAX) return NULL;
    const int q = w * 4;

    // consider: provision local rnd-seed

    checkers_arg * da = malloc(sizeof *da + q * 3);
    real r;
    spherical(n, &r, &da->theta, &da->phi);
    da->r = 1 / r;
    da->o = o;
    da->q = w;
    da->reflection_filter = reflection_filter;
    da->absorption_filter = absorption_filter;
    da->refraction_filter = refraction_filter;
    beams(da->data, q, w);
    beams(da->data + q, q, w);
    beams(da->data + q * 2, q, w);
    *df = checkers_decoration;
    return da;
}
