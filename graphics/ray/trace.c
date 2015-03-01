//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "trace.h"
#include "xmath.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct {
    scene * scene;
    scene_sky sky;
    light_spot * spots;
    int spot_count;
} world__;

    world
alloc_world(int count)
{
    assert(count > 0);
    scene * sc = calloc(1, scene_size(count));
    sc->object_count = count;
    world__ * w = malloc(sizeof *w);
    w->scene = sc;
    w->sky = white_sky;
    return w;
}

    void
set_object(world world_, int i, scene_object object)
{
    world__ * w = world_;
    assert(i >= 0 && i < w->scene->object_count);
    w->scene->objects[i] = object;
}

    void
set_sky(world world_, scene_sky sky)
{
    world__ * w = world_;
    w->sky = sky;
}

    void
set_spots(world world_, light_spot * spots, int count)
{
    world__ * w = world_;
    w->spots = spots;
    w->spot_count = count;

}

    void
destroy_world(world world_)
{
    world__ * w = world_;
    free(w->scene);
    free(w);
}

typedef struct {
    color lens;
    ray ray;
    int hop;
    bitarray * inside;
} detector;

static color trace__(const detector *, world__ *);

    color
trace(ray t, world world_)
{
    world__ * w = world_;
    detector detector_ = {
        {1, 1, 1},
        .ray = t,
        .hop = max_hops};
    const size_t q = w->scene->object_count;
    detector_.inside = calloc(1, ba_size(q));
    detector_.inside->bit_count = q;
    init_inside(detector_.inside, w->scene, &t);
    if (debug && ba_firstset(detector_.inside) >= 0)
        fprintf(stderr, "initial detector is at inside of %d\n",
                ba_firstset(detector_.inside));
    const color detected = trace__(&detector_, w);
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
trace_hop(ray t, color filter_,
        const detector * detector_, world__ * w)
{
    detector t_detector = *detector_;
    t_detector.ray = t;
    t_detector.hop --;
    filter(&t_detector.lens, filter_);
    color detected = trace__(&t_detector, w);
    filter(&detected, filter_);
    return detected;
}

    static void
passthrough_filter(color * color_, const object_optics * so,
        real distance_)
{
    const color so_filter = so->passthrough_filter;
    color filter_;
    if (distance_ >= HUGE_REAL / 2) {
        color f = so_filter;
        if ( ! is_near(f.r, 1)) f.r = 0;
        if ( ! is_near(f.g, 1)) f.g = 0;
        if ( ! is_near(f.b, 1)) f.b = 0;
        filter_ = f;
    } else {
        filter_ = (color){
            rpow(so_filter.r, distance_),
            rpow(so_filter.g, distance_),
            rpow(so_filter.b, distance_)
        };
    }
    filter(color_, filter_);
}

    static color
spot_absorption(const ray * surface, const object_optics * so,
        const world__ * w, bitarray * inside)
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
        if (NULL == closest_surface(&s, w->scene, inside, NULL)) {
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
        const detector * detector_, world__ * w)
{
    color detected = {0, 0, 0};
    const ptrdiff_t i = so - w->scene->objects;
    int outside_i = ba_firstset(detector_->inside);
    const bool enters = (outside_i != i);
    ba_assign(detector_->inside, i, enters);
    if ( ! enters) outside_i = ba_firstset(detector_->inside);
    real outside_refraction_index = (outside_i >= 0)
        ? w->scene->objects[outside_i].optics.refraction_index
        : 1;
    if (outside_refraction_index <= 0) {
        if (debug) {
            if (detector_->hop != max_hops) /* (view _can_ happen to be inside) */
                fprintf(stderr, "we got inside opaque object %d\n", (int)i);
        }
        goto out;
    }
    real refraction_index = outside_refraction_index
            / so->optics.refraction_index;
    if (refraction_index <= 0) {
        if (debug && ! enters && detector_->hop != max_hops) {
            fprintf(stderr, "hit objects[%d] from inside of surface"
                    ", and has no refraction.  how did we enter it?\n", (int)i);
        }
        goto out;
    }
    if ( ! enters) {
        refraction_index = 1 / refraction_index;
        scale(&ray_.head, -1);
    }
    ray_.head = refraction(ray_.head, detector_->ray.head,
            refraction_index, TINY_REAL);
    if (is_DISORIENTED(&ray_.head)) {
        goto out;
    }
    if (verbose) fprintf(stderr, "refraction\n");
    const color refraction_filter =
        (transparent_refraction_on_equal_index && is_near(refraction_index, 1))
        ? (color){1, 1, 1}
        : so->optics.refraction_filter;
    detected = trace_hop(ray_, refraction_filter, detector_, w);
out:
    ba_assign(detector_->inside, i, ( ! enters));
    return detected;
}

    static color
trace__(const detector * detector_, world__ * w)
{
    if (0 == detector_->hop || ignorable_color(detector_->lens))
        return (color){0, 0, 0};

    ray surface = detector_->ray;
    assert(is_near(length(surface.head), 1));
    stack toggled = EMPTY_STACK;
    const int detector_inside_i = ba_firstset(detector_->inside);
    const scene_object * closest_object
        = closest_surface(&surface, w->scene, detector_->inside, &toggled);
    if (closest_object == NULL) {
        int rgb_clear = 0;
        const int adinf_i = detector_inside_i;
        if (adinf_i >= 0) {
            const color * f = &w->scene->objects[adinf_i].optics
                .passthrough_filter;
            if ( ! is_near(f->r, 1)) rgb_clear |= 0x1;
            if ( ! is_near(f->g, 1)) rgb_clear |= 0x2;
            if ( ! is_near(f->b, 1)) rgb_clear |= 0x4;
            if (rgb_clear == 0x7) return (color){0, 0, 0};
        }
        color _ = (eliminate_direct_sky && detector_->hop == max_hops)
            ? DIRECT_SKY
            : w->sky(detector_->ray.head);
        if (rgb_clear) {
            if (rgb_clear & 0x1) _.r = 0;
            if (rgb_clear & 0x2) _.g = 0;
            if (rgb_clear & 0x4) _.b = 0;
        }
        return _;
    } else {
        const ptrdiff_t i = closest_object - w->scene->objects;
        assert(i >= 0 && i < w->scene->object_count);
        const bool exits = 0 < scalar_product(
                surface.head, detector_->ray.head);
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
                color filter_ = optics->reflection_filter;
                filter_.r *= reflection_on_inside;
                filter_.g *= reflection_on_inside;
                filter_.b *= reflection_on_inside;
                const ray in_reflection_ = {
                    .endpoint = surface.endpoint,
                    .head = reflection(normal_, detector_->ray.head) };
                if (verbose) fprintf(stderr, "reflection (from inside)\n");
                detected = trace_hop(in_reflection_, filter_, detector_, w);
            }
        } else {
            if (debug && ba_isset(detector_->inside, i))
                fprintf(stderr, "hit objects[%d] from outside of surface"
                        ", but book-keeping says inside of it\n", (int)i);
            const ray reflection_ = {
                .endpoint = surface.endpoint,
                .head = reflection(surface.head, detector_->ray.head) };
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
        if (optics->refraction_index > 0) {
            const color refraction_color = refraction_trace(
                    surface, closest_object, detector_, w);
            detected = optical_sum(detected, refraction_color);
        }
        if (detector_inside_i >= 0) {
            scene_object * io = w->scene->objects + detector_inside_i;
            passthrough_filter(
                    &detected, &io->optics,
                    distance(detector_->ray.endpoint, surface.endpoint));
        }
        int toggled_i;
        while ((toggled_i = st_pop(&toggled)) != STACK_VOID)
            ba_toggle(detector_->inside, toggled_i);

        return detected;
    }
}

