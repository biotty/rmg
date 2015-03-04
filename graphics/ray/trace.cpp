//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "trace.h"
#include "xmath.h"
#include <cassert>
#include <iostream>


struct detector {
    color lens;
    ray ray_;
    int hop;
    bitarray * inside;
};


static color ray_trace(const detector *, world *);


    color
trace(ray t, world * w)
{
    const size_t q = w->scene_.object_count;
    detector detector_ = {{1, 1, 1}, t, max_hops,
        (bitarray *)calloc(1, ba_size(q))};
    detector_.inside->bit_count = q;
    init_inside(detector_.inside, w->scene_, &t);
    if (debug && ba_firstset(detector_.inside) >= 0)
        std::cerr << "initial detector is at inside of "
                << ba_firstset(detector_.inside) << std::endl;
    const color detected = ray_trace(&detector_, w);
    free(detector_.inside);
    return detected;
}

    static inline bool
ignorable_color(const color lens)
{
    const real small = 0.1;
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
passthrough_apply(color * color_, const object_optics * so,
        real distance_)
{
    const compact_color so_filter = so->passthrough_filter;
    const compact_color filter_ = {
        static_cast<unsigned char>(255*rpow(so_filter.r/(real)255, distance_)),
        static_cast<unsigned char>(255*rpow(so_filter.g/(real)255, distance_)),
        static_cast<unsigned char>(255*rpow(so_filter.b/(real)255, distance_))
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
    class scoped_bit_toggler {
        bitarray * a; int i; bool enters; public:
        scoped_bit_toggler(bitarray * a, int i, bool enters)
            : a(a), i(i), enters(enters) { ba_assign(a, i, enters); }
        ~scoped_bit_toggler() { ba_assign(a, i, !enters); }
    };
    scoped_bit_toggler sbt(detector_->inside, i, enters);
    if ( ! enters) outside_i = ba_firstset(detector_->inside);
    unsigned outside_refraction_index_micro = 1000000;
    if (outside_i >= 0) {
        outside_refraction_index_micro
            = w->scene_.objects[outside_i].optics.refraction_index_micro;
        if (0 == outside_refraction_index_micro) {
            if (debug) {
                if (detector_->hop != max_hops) /* (view _can_ happen to be inside) */
                    std::cerr << "we got inside opaque object " << i << std::endl;
            }
            return detected;
        }
    }
    real refraction_index = outside_refraction_index_micro
            /(real) so->optics.refraction_index_micro;
    if ( ! enters) {
        refraction_index = 1 / refraction_index;
        scale(&ray_.head, -1);
    }
    ray_.head = refraction(ray_.head, detector_->ray_.head,
            refraction_index, TINY_REAL);
    if (is_DISORIENTED(&ray_.head)) {
        return detected;
    }
    compact_color refraction_filter = {255, 255, 255};
    if ( ! transparent_refraction_on_equal_index
            || ! is_near(refraction_index, 1))
        refraction_filter = so->optics.refraction_filter;
    detected = trace_hop(ray_, refraction_filter, detector_, w);
    return detected;
}

    static color
ray_trace(const detector * detector_, world * w)
{
    color detected = {0, 0, 0};
    if (0 == detector_->hop || ignorable_color(detector_->lens))
        return detected;

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
            if (rgb_clear == 0x7) return detected;
        }
        detected = DIRECT_SKY;
        if ( ! eliminate_direct_sky
                || detector_->hop != max_hops)
            detected = w->sky(detector_->ray_.head);
        if (rgb_clear) {
            if (rgb_clear & 0x1) detected.r = 0;
            if (rgb_clear & 0x2) detected.g = 0;
            if (rgb_clear & 0x4) detected.b = 0;
        }
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
        if (exits) {
            if (debug && ! ba_isset(detector_->inside, i))
                std::cerr << "hit from inside of surface "
                        "but book-keeped as outside-of " << i << std::endl;
            if (reflection_on_inside > 0) {
                direction normal_ = surface.head;
                scale(&normal_, -1);
                compact_color filter_ = optics->reflection_filter;
                filter_.r *= reflection_on_inside;
                filter_.g *= reflection_on_inside;
                filter_.b *= reflection_on_inside;
                const ray in_reflection_ = { surface.endpoint,
                    reflection(normal_, detector_->ray_.head) };
                detected = trace_hop(in_reflection_, filter_, detector_, w);
            }
        } else {
            if (debug && ba_isset(detector_->inside, i))
                std::cerr << "hit from outside of surface "
                        "but book-keeped as inside-of " << i << std::endl;
            const ray reflection_ = { surface.endpoint,
                reflection(surface.head, detector_->ray_.head) };
            detected = trace_hop(
                    reflection_, optics->reflection_filter,
                    detector_, w);
        }
        const int inside_i = ba_firstset(detector_->inside);
        if (inside_i < 0) {
            const color absorbed = spot_absorption(
                    &surface, optics, w, detector_->inside);
            color_add(&detected, absorbed);
        }
        if (optics->refraction_index_micro) {
            const color refraction_color = refraction_trace(
                    surface, closest_object, detector_, w);
            color_add(&detected, refraction_color);
        }
        if (detector_inside_i >= 0) {
            scene_object * io = w->scene_.objects + detector_inside_i;
            passthrough_apply(&detected, &io->optics,
                    distance(detector_->ray_.endpoint, surface.endpoint));
        }
        int toggled_i;
        while ((toggled_i = st_pop(&toggled)) != STACK_VOID)
            ba_toggle(detector_->inside, toggled_i);
    }
    return detected;
}
