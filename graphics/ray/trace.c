//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "trace.h"
#include "xmath.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


typedef struct {
    color lens;
    ray ray_;
    int hop;
    bitarray * inside;
} detector;

static color ray_trace(const detector *, world *);

    color
trace(ray t, world * w)
{
    const size_t q = w->scene_.object_count;
    detector detector_ = { {1, 1, 1}, t, max_hops,
        (bitarray *)calloc(1, ba_size(q)) };
    detector_.inside->bit_count = q;
    init_inside(detector_.inside, w->scene_, &t);
    if (debug && ba_firstset(detector_.inside) >= 0)
        fprintf(stderr, "initial detector is at inside of %d\n",
                ba_firstset(detector_.inside));
    const color detected = ray_trace(&detector_, w);
    free(detector_.inside);
    return detected;
}

    static inline bool
ignorable_color(const color lens)
{
    const real small = 2.0/256;
    return (lens.r < small && lens.g < small && lens.b < small);
}

    static color
trace_hop(ray t, compact_color filter_,
        const detector * detector_, world * w)
{
    detector t_detector = *detector_;
    t_detector.ray_ = t;
    t_detector.hop --;
    filter(&t_detector.lens, filter_);
    color detected = ray_trace(&t_detector, w);
    filter(&detected, filter_);
    return detected;
}

    static void
passthrough_filter(color * color_, const object_optics * so,
        real distance_)
{
    const compact_color so_filter = so->passthrough_filter;
    const compact_color filter_ = {
        255 * rpow(so_filter.r/(real)255, distance_),
        255 * rpow(so_filter.g/(real)255, distance_),
        255 * rpow(so_filter.b/(real)255, distance_)
    };
    filter(color_, filter_);
}

    static color
spot_absorption(const ray * surface, const object_optics * so,
        const world * w, bitarray * inside)
{
    color sum_ = {0, 0, 0};
    for (int i=0; i<w->spot_count; i++) {
        const light_spot * lamp = &w->spots[i];
        color color_ = lamp->light;
        filter(&color_, so->absorption_filter);
        if (ignorable_color(color_)) continue;
        direction to_spot = distance_vector(surface->endpoint, lamp->spot);
        normalize(&to_spot);
        const real a = scalar_product(surface->head, to_spot);
        if (a <= 0) continue;
        ray s = { .endpoint = surface->endpoint, .head = to_spot };
        if (NULL == closest_surface(&s, w->scene_, inside, NULL)) {
            const real ua = acos(1 - a) * (2/REAL_PI);
            sum_.r += color_.r * ua;
            sum_.g += color_.g * ua;
            sum_.b += color_.b * ua;
        }
    }
    return sum_;
}

    static color
refraction_trace(ray ray_, const scene_object * so,
        const detector * detector_, world * w)
{
    color detected = {0, 0, 0};
    const ptrdiff_t i = so - w->scene_.objects;
    int outside_i = ba_firstset(detector_->inside);
    const bool enters = (outside_i != i);
    ba_assign(detector_->inside, i, enters);
#define RETURN { ba_assign(detector_->inside, i, ( ! enters)); \
                 return detected; }
    if ( ! enters) outside_i = ba_firstset(detector_->inside);
    real outside_refraction_index = 1;
    if (outside_i >= 0)
        outside_refraction_index = w->scene_.objects[outside_i]
            .optics.refraction_index_micro /(real) 1000000;
    if (outside_refraction_index <= 0) {
        if (debug) {
            if (detector_->hop != max_hops) /* (view _can_ happen to be inside) */
                fprintf(stderr, "we got inside opaque object %d\n", (int)i);
        }
        RETURN
    }
    real refraction_index = outside_refraction_index
            * 1000000 /(real) so->optics.refraction_index_micro;
    if ( ! enters) {
        refraction_index = 1 / refraction_index;
        scale(&ray_.head, -1);
    }
    ray_.head = refraction(ray_.head, detector_->ray_.head,
            refraction_index, TINY_REAL);
    if (is_DISORIENTED(&ray_.head)) {
        RETURN
    }
    if (verbose) fprintf(stderr, "refraction\n");
    compact_color refraction_filter = {255, 255, 255};
    if ( ! transparent_refraction_on_equal_index
            || ! is_near(refraction_index, 1))
        refraction_filter = so->optics.refraction_filter;
    detected = trace_hop(ray_, refraction_filter, detector_, w);
    RETURN
#undef RETURN
}

    static color
ray_trace(const detector * detector_, world * w)
{
    if (0 == detector_->hop || ignorable_color(detector_->lens))
        return (color){0, 0, 0};

    ray surface = detector_->ray_;
    assert(is_near(length(surface.head), 1));
    stack toggled = EMPTY_STACK;
    const int detector_inside_i = ba_firstset(detector_->inside);
    const scene_object * closest_object
        = closest_surface(&surface, w->scene_, detector_->inside, &toggled);
    if (closest_object == NULL) {
        int rgb_clear = 0;
        const int adinf_i = detector_inside_i;
        if (adinf_i >= 0) {
            compact_color f
                = w->scene_.objects[adinf_i].optics.passthrough_filter;
            if (f.r != 255) rgb_clear |= 0x1;
            if (f.g != 255) rgb_clear |= 0x2;
            if (f.b != 255) rgb_clear |= 0x4;
            if (rgb_clear == 0x7) return (color){0, 0, 0};
        }
        color ret = DIRECT_SKY;
        if ( ! eliminate_direct_sky
                || detector_->hop != max_hops)
            ret = w->sky(detector_->ray_.head);
        if (rgb_clear) {
            if (rgb_clear & 0x1) ret.r = 0;
            if (rgb_clear & 0x2) ret.g = 0;
            if (rgb_clear & 0x4) ret.b = 0;
        }
        return ret;
    } else {
        const ptrdiff_t i = closest_object - w->scene_.objects;
        assert(i >= 0 && i < w->scene_.object_count);
        const bool exits = 0 < scalar_product(
                surface.head, detector_->ray_.head);
        const object_optics * optics = &closest_object->optics;
        object_optics auto_store;
        if (closest_object->decoration) {
            closest_object->decoration(&surface,
                    closest_object->decoration_arg,
                    &auto_store, &closest_object->optics);
            optics = &auto_store;
        }
        color detected = {0, 0, 0};
        if (exits) {
            if (debug && ! ba_isset(detector_->inside, i))
                fprintf(stderr, "hit objects[%d] from inside of surface"
                        ", but we never took note of entering it\n", (int)i);
            if (reflection_on_inside > 0) {
                direction normal_ = surface.head;
                scale(&normal_, -1);
                compact_color filter_ = optics->reflection_filter;
                filter_.r *= reflection_on_inside;
                filter_.g *= reflection_on_inside;
                filter_.b *= reflection_on_inside;
                const ray in_reflection_ = { surface.endpoint,
                    reflection(normal_, detector_->ray_.head) };
                if (verbose) fprintf(stderr, "reflection (from inside)\n");
                detected = trace_hop(in_reflection_, filter_, detector_, w);
            }
        } else {
            if (debug && ba_isset(detector_->inside, i))
                fprintf(stderr, "hit objects[%d] from outside of surface"
                        ", but book-keeping says inside of it\n", (int)i);
            const ray reflection_ = { surface.endpoint,
                reflection(surface.head, detector_->ray_.head) };
            if (verbose) fprintf(stderr, "reflection\n");
            detected = trace_hop(
                    reflection_, optics->reflection_filter,
                    detector_, w);
        }
        const int inside_i = ba_firstset(detector_->inside);
        if (inside_i < 0) {
            if (verbose) fprintf(stderr, "absorption\n");
            const color absorbed = spot_absorption(
                    &surface, optics, w, detector_->inside);
            detected = optical_sum(detected, absorbed);
        }
        if (optics->refraction_index_micro) {
            const color refraction_color = refraction_trace(
                    surface, closest_object, detector_, w);
            detected = optical_sum(detected, refraction_color);
        }
        if (detector_inside_i >= 0) {
            scene_object * io = w->scene_.objects + detector_inside_i;
            passthrough_filter(
                    &detected, &io->optics,
                    distance(detector_->ray_.endpoint, surface.endpoint));
        }
        int toggled_i;
        while ((toggled_i = st_pop(&toggled)) != STACK_VOID)
            ba_toggle(detector_->inside, toggled_i);

        return detected;
    }
}
