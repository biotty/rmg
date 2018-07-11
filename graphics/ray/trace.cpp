//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "trace.hpp"
#include "scene.hpp"
#include "bitarray.hpp"
#include "stack.hpp"
#include <cassert>
#include <iostream>


struct detector {
    color lens;
    ray ray_;
    int hop;
    mutable bitarray inside;

    detector(ray t, int q) : lens{1, 1, 1}, ray_(t), hop(max_hops), inside(q) {}
};


static color ray_trace(const detector &, const world &);


    color
trace(ray t, const world & w)
{
    const size_t q = w.scene_.size();
    detector detector_(t, q);
    init_inside(detector_.inside, w.scene_, &t);
    if (debug && detector_.inside.firstset() >= 0)
        std::cerr << "initial detector is at inside of "
                << detector_.inside.firstset() << std::endl;
    const color detected = ray_trace(detector_, w);
    return detected;
}

    static inline bool
ignorable_color(const color lens)
{
    const real small = 1 / (real)256;
    return (lens.r < small && lens.g < small && lens.b < small);
}

    static color
trace_hop(ray t, compact_color filter_,
        const detector & detector_, const world & w)
{
    detector t_detector = detector_;
    t_detector.ray_ = t;
    t_detector.hop --;
    filter(&t_detector.lens, filter_);
    color detected = ray_trace(t_detector, w);
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
        const world & w, bitarray & inside)
{
    color sum_ = {0, 0, 0};
    const size_t n = w.spots_.size();
    for (size_t i = 0; i < n; i++) {
        const light_spot & ls = w.spots_[i];
        color color_ = ls.light;
        filter(&color_, so->absorption_filter);
        if (ignorable_color(color_)) continue;
        direction to_spot = distance_vector(surface->endpoint, ls.spot);
        normalize(&to_spot);
        // consider: inv-scale with up to |to_spot|^2 (global setting 0-1)
        const real a = scalar_product(surface->head, to_spot);
        if (a <= 0) continue;
        ray s = { surface->endpoint, to_spot };
        if (nullptr == closest_surface(w.scene_, &s, inside, nullptr)) {
            const real ua = acos(1 - a) * (2/REAL_PI);
            sum_.r += color_.r * ua;
            sum_.g += color_.g * ua;
            sum_.b += color_.b * ua;
        }
    }
    return sum_;
}

    static color
reflection_trace(compact_color reflection_filter, ray ray_, bool exits, int i,
        const detector & detector_, const world & w)
{
    direction normal_ = ray_.head;
    if (exits) {
        scale(&normal_, -1);
        if (debug && ! detector_.inside.isset(i))
            std::cerr << "hit from inside of surface "
                "but book-keeped as outside-of " << i << std::endl;
    } else {
        if (debug && detector_.inside.isset(i))
            std::cerr << "hit from outside of surface "
                "but book-keeped as inside-of " << i << std::endl;
    }
    ray reflection_ = { ray_.endpoint,
        reflection(normal_, detector_.ray_.head) };

    return trace_hop(reflection_, reflection_filter,
            detector_, w);
}


enum refraction_ret {
    opaque,
    reflect,
    total_reflect,
    transparent,
};

    static enum refraction_ret
refraction_trace(ray ray_, const scene_object * so,
        float optics_refraction_index,
        compact_color optics_refraction_filter,
        const detector & detector_, const world & w, color * result)
{
    const ptrdiff_t i = so - &w.scene_[0];
    int outside_i = detector_.inside.firstset();
    const bool enters = (outside_i != i);

    class scoped_bit_flipper {
        bitarray & a; const int i; const bool enters;
        public:
        scoped_bit_flipper(bitarray & a, int i, bool enters)
            : a(a), i(i), enters(enters)
        {
            a.assign(i, enters);
        }
        ~scoped_bit_flipper()
        {
            a.assign(i, !enters);
        }
    };
    scoped_bit_flipper sbf(detector_.inside, i, enters);

    if ( ! enters) outside_i = detector_.inside.firstset();
    float outside_refraction_index = 1.0;
    if (outside_i >= 0) {
        outside_refraction_index
            = w.scene_[outside_i].optics.refraction_index;
        if (0 == outside_refraction_index) {
            if (debug) {
                if (detector_.hop != max_hops) /* (view _can_ happen to be inside) */
                    std::cerr << "we got inside opaque object " << i << std::endl;
            }
            return opaque;
        }
    }
    if (transparent_on_equal_index
            && so->decoration == nullptr
            && outside_refraction_index
            == optics_refraction_index) {
        ray_.head = detector_.ray_.head;
        compact_color transparent_ = {255, 255, 255};
        *result = trace_hop(ray_, transparent_, detector_, w);
        return transparent;
    }
    real refraction_index;
    if (enters) {
        refraction_index = outside_refraction_index / optics_refraction_index;
    } else {
        refraction_index = optics_refraction_index / outside_refraction_index;
        scale(&ray_.head, -1);
    }
    ray_.head = refraction(ray_.head, detector_.ray_.head, refraction_index);
    if (is_DISORIENTED(&ray_.head)) {
        return total_reflect;
    }
    // consider: light density changes with angle, so that
    //           we must scale the result proportionally.
    *result = trace_hop(ray_, optics_refraction_filter, detector_, w);
    return reflect;
}

    static color
ray_trace(const detector & detector_, const world & w)
{
    color detected = {0, 0, 0};
    if (ignorable_color(detector_.lens)) return detected;
    if (0 == detector_.hop) {
        if (debug) std::cerr << "no more hops" << std::endl;
        return detected;
    }
    ray surface = detector_.ray_;
    assert(is_near(length(surface.head), 1));
    stack flips;
    const int det_inside_i = detector_.inside.firstset();
    const scene_object * closest_object
        = closest_surface(w.scene_, &surface, detector_.inside, &flips);
    if ( ! closest_object) {
        const int adinf_i = det_inside_i;
        if (adinf_i >= 0) {
            compact_color f
                = w.scene_[adinf_i].optics.passthrough_filter;
            if (f.r != 255 || f.g != 255 || f.b != 255) {
                return detected;
            }
        }
        return eliminate_direct_sky && detector_.hop == max_hops
            ? DIRECT_SKY : w.sky(detector_.ray_.head);
    }

    const ptrdiff_t i = closest_object - &w.scene_[0];
    assert(i >= 0 && i < static_cast<ptrdiff_t>(w.scene_.size()));
    const bool exits = 0 < scalar_product(
            surface.head, detector_.ray_.head);
    const object_optics * optics = &closest_object->optics;
    object_optics auto_store;
    if (closest_object->decoration) {
        closest_object->decoration(&surface,
                closest_object->decoration_arg,
                &auto_store, &closest_object->optics);
        optics = &auto_store;
    }
    const int inside_i = detector_.inside.firstset();
    if (inside_i < 0) {
        const color absorbed = spot_absorption(
                &surface, optics, w, detector_.inside);
        // consider: function of angle so that up at angle_max
        //           gives 1/cos(a) factor and stays there up
        //           to a right angle.
        color_add(&detected, absorbed);
    }
    enum refraction_ret r = opaque;
    compact_color reflection_filter = optics->reflection_filter;
    if (optics->refraction_index) {
        color refraction_color;
        r = refraction_trace(
                surface, closest_object,
                optics->refraction_index,
                optics->refraction_filter,
                detector_, w, &refraction_color);
        if (r == reflect || r == transparent) {
            color_add(&detected, refraction_color);
        } else if (r == total_reflect) {
            saturated_add(&reflection_filter, optics->refraction_filter);
        }
    }
    if (r != transparent) {
        const color reflected = reflection_trace(reflection_filter,
                surface, exits, i, detector_, w);
        color_add(&detected, reflected);
    }
    if (det_inside_i >= 0) {
        const scene_object * io = &w.scene_[det_inside_i];
        passthrough_apply(&detected, &io->optics,
                distance(detector_.ray_.endpoint, surface.endpoint));
    }
    int flipped_i;
    while ((flipped_i = flips.pop()) >= 0)
        detector_.inside.flip(flipped_i);

    return detected;
}
