//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "map.h"
#include "math.h"
#include "photo.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    photo * photo;
    map_application a;
    real r;
    real theta;
    real phi;
} map_arg;

typedef struct {
    photo * photo;
    map_application a;
    real r;
    real theta;
    real phi;
} pmap_arg;

typedef struct {
    photo * photo;
    map_application a;
    point o;
    real r;
    real theta;
    real phi;
} omap_arg;

typedef struct {
    photo * photo;
    map_application a;
    point o;
    real r;
    real theta;
    real phi;
} lmap_arg;

    void
generic_map_delete(void * decoration_arg)
{
    map_arg * da = decoration_arg;
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
map_apply__(const map_application * a, const photo * ph, real x, real y,
        scene_object * so)
{
    if ( ! ph) {
        so->reflection_filter = a->reflection_adjust;
        so->absorption_filter = a->absorption_adjust;
        so->refraction_index = a->refraction_index;
        so->refraction_filter = a->refraction_adjust;
        so->traversion_filter = a->traversion_adjust;
        return;
    }
    const color c = photo_color(ph, x * ph->width, y * ph->height);
    so->reflection_filter = linear(c, a->reflection_factor, a->reflection_adjust);
    so->absorption_filter = linear(c, a->absorption_factor, a->absorption_adjust);
    so->refraction_index = a->refraction_index;
    so->refraction_filter = linear(c, a->refraction_factor, a->refraction_adjust);
    so->traversion_filter = linear(c, a->traversion_factor, a->traversion_adjust);
}

    static void
zoom_(const real r, real * x_, real * y_)
{
    *x_ = zoom(r, *x_);
    *y_ = zoom(r, *y_);
}

    static void
wrap_(const map_application * a, real * x, real * y)
{
    *x += a->x_wrap;
    if (*x >= 1) *x -= 1;
    *y += a->y_wrap;
    if (*y >= 1) *y -= 1;
}

    static void
map__(const ray * ray_, scene_object * so)
{
    const map_arg * da = so->decoration_arg;
    real x, y;
    direction d = inverse_rotation(ray_->head, da->theta, da->phi);
    direction_to_unitsquare(&d, &x, &y);
    zoom_(da->r, &x, &y);
    wrap_(&da->a, &x, &y);
    map_apply__(&da->a, da->photo, x, y, so);
}

    static void
pmap__(const ray * ray_, scene_object * so)
{
    const pmap_arg * da = so->decoration_arg;
    direction d = inverse_rotation(
            direction_from_origo(ray_->endpoint),
            da->theta, da->phi);
    real x = d.x;
    real y = d.y;
    zoom_(da->r, &x, &y);
    wrap_(&da->a, &x, &y);
    map_apply__(&da->a, da->photo, x, y, so);
}

    static void
omap__(const ray * ray_, scene_object * so)
{
    const omap_arg * da = so->decoration_arg;
    direction d = inverse_rotation(
            distance_vector(da->o, ray_->endpoint),
            da->theta, da->phi);
    real x, y;
    direction_to_unitsquare(&d, &x, &y);
    zoom_(da->r, &x, &y);
    wrap_(&da->a, &x, &y);
    map_apply__(&da->a, da->photo, x, y, so);
}

    static void
lmap__(const ray * ray_, scene_object * so)
{
    static const real pi = REAL_PI;
    static const real two_pi = REAL_PI * 2;
    const lmap_arg * da = so->decoration_arg;
    direction d = inverse_rotation(
            distance_vector(da->o, ray_->endpoint),
            da->theta, da->phi);
    real x = (ratan(d.y, d.x) + pi) / two_pi;
    real y = d.z * da->r;
    y -= rfloor(y);
    if (y == 1) y = 0;
    wrap_(&da->a, &x, &y);
    map_apply__(&da->a, da->photo, x, y, so);
}

    void *
map_decoration(
        object_decoration * df, const n_map_setup * setup)
{
    map_arg * da = malloc(sizeof *da);
    da->a = setup->a;
    da->photo = photo_create(setup->path);
    real r;
    spherical(setup->n, &r, &da->theta, &da->phi);
    da->r = 1 / r;
    *df = map__;
    return da;
}

    void *
pmap_decoration(
        object_decoration * df, const n_map_setup * setup)
{
    pmap_arg * da = malloc(sizeof *da);
    da->a = setup->a;
    da->photo = photo_create(setup->path);
    real r;
    spherical(setup->n, &r, &da->theta, &da->phi);
    da->r = 1 / r;
    *df = pmap__;
    return da;
}

    void *
omap_decoration(
        object_decoration * df, const n_o_map_setup * setup)
{
    omap_arg * da = malloc(sizeof *da);
    da->a = setup->a;
    da->photo = photo_create(setup->path);
    real r;
    spherical(setup->n, &r, &da->theta, &da->phi);
    da->r = 1 / r;
    da->o = setup->o;
    *df = omap__;
    return da;
}

    void *
lmap_decoration(
        object_decoration * df, const n_o_map_setup * setup)
{
    lmap_arg * da = malloc(sizeof *da);
    da->a = setup->a;
    da->photo = photo_create(setup->path);
    real r;
    spherical(setup->n, &r, &da->theta, &da->phi);
    da->r = 1 / r;
    da->o = setup->o;
    *df = lmap__;
    return da;
}

