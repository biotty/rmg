//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "trace.hpp"
#include "scene.hpp"
#include "bitarray.hpp"
#include <cassert>
#include <iostream>

world::world(scene_sky sky, del_f inter_f, del_f decoration_f)
    : sky(sky), del_inter(inter_f), del_decoration(decoration_f)
{}

world::~world() {
    for (void * d : decoration_args) del_decoration(d);
    for (void * a : inter_args) del_inter(a);
    for (void * o : object_args) free(o);
}

struct detector {
    int hop;
    color lens;
    bitarray inside;
};

static color ray_trace(detector &, ray, const world &);


    color
trace(ray t, const world & w)
{
    const size_t q = w.scene_.size();
    detector detector_{max_hops, {1, 1, 1}, q};
    init_inside(detector_.inside, w.scene_, &t);
    if (debug && detector_.inside.firstset() >= 0)
        std::cerr << "initial detector is at inside of "
                << detector_.inside.firstset() << "\n";
    const color detected = ray_trace(detector_, t, w);
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
        detector /*copy*/ detector_, const world & w)
{
    detector_.hop--;
    filter(&detector_.lens, filter_);
    color detected = ray_trace(detector_, t, w);
    filter(&detected, filter_);
    // improve: non-compact filter for detector and passthrough
    //          only when filtering from surface data directly.
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

static const color black = {0, 0, 0};

    static color
spot_absorption(const ray & surface, const object_optics * so,
        const world & w, bitarray & inside)
{
    color sum_ = black;
    const size_t n = w.spots_.size();
    for (size_t i = 0; i < n; i++) {
        const light_spot & ls = w.spots_[i];
        color color_ = ls.light;
        filter(&color_, so->absorption_filter);
        if (ignorable_color(color_)) continue;
        direction to_spot = distance_vector(surface.endpoint, ls.spot);
        normalize(&to_spot);
        // consider: inv-scale with up to |to_spot|^2 (global setting 0-1)
        const real a = scalar_product(surface.head, to_spot);
        if (a <= 0) continue;
        ray s = { surface.endpoint, to_spot };
        if ( ! closest_surface(w.scene_, &s, inside)) {
            const real ua = acos(1 - a) * (2/REAL_PI);
            sum_.r += color_.r * ua;
            sum_.g += color_.g * ua;
            sum_.b += color_.b * ua;
        }
    }
    return sum_;
}

    static color
reflection_trace(compact_color reflection_filter, ray ray_,
        detector & detector_, direction det_head, const world & w)
{
    ray reflection_{ray_.endpoint, reflection(ray_.head, det_head)};
    return trace_hop(reflection_, reflection_filter, detector_, w);
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
        detector & detector_, direction det_head,
        const world & w, color * result)
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
                if (detector_.hop != max_hops)  // <-- view MAY be inside
                    std::cerr << "we got inside opaque object " << i << "\n";
            }
            return opaque;
        }
    }
    if (transparent_on_equal_index
            && so->decoration == nullptr
            && outside_refraction_index
            == optics_refraction_index) {
        ray_.head = det_head;
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
    ray_.head = refraction(ray_.head, det_head, refraction_index);
    if (is_DISORIENTED(&ray_.head)) {
        return total_reflect;
    }
    *result = trace_hop(ray_, optics_refraction_filter, detector_, w);
    return reflect;
}

    static color
sky(detector & detector_, direction d, scene_sky f)
{
    if (eliminate_direct_sky && detector_.hop == max_hops)
        return DIRECT_SKY;

    return f(d);
}

    static color
ray_trace(detector & detector_, ray t, const world & w)
{
    color detected = black;
    if (ignorable_color(detector_.lens)) return detected;
    if (0 == detector_.hop) {
        if (debug) std::cerr << "no more hops\n";
        return detected;
    }
    ray surface = t;
    assert(is_near(length(surface.head), 1));
    const int det_inside_i = detector_.inside.firstset();
    const scene_object * closest_object
        = closest_surface(w.scene_, &surface, detector_.inside);
    if ( ! closest_object) {
        const int adinf_i = det_inside_i;
        if (adinf_i >= 0) {
            compact_color f
                = w.scene_[adinf_i].optics.passthrough_filter;
            if (f.r != 255 || f.g != 255 || f.b != 255) {
                return detected;
            }
        }
        return sky(detector_, t.head, w.sky);
    }

    const ptrdiff_t i = closest_object - &w.scene_[0];
    assert(i >= 0 && i < static_cast<ptrdiff_t>(w.scene_.size()));
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
                surface, optics, w, detector_.inside);
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
                detector_, t.head, w, &refraction_color);
        if (r == reflect || r == transparent) {
            color_add(&detected, refraction_color);
        } else if (r == total_reflect) {
            saturated_add(&reflection_filter, optics->refraction_filter);
        }
    }
    if (r != transparent) {
        const color reflected = reflection_trace(reflection_filter,
                surface, detector_, t.head, w);
        color_add(&detected, reflected);
    }
    if (det_inside_i >= 0) {
        const scene_object * io = &w.scene_[det_inside_i];
        passthrough_apply(&detected, &io->optics,
                distance(t.endpoint, surface.endpoint));
    }

    return detected;
}
