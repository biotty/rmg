//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "scene.h"
#include "trace.hpp"
#include "bitarray.hpp"
#include "stack.hpp"

    void
init_inside(bitarray & inside, scene const & s, const ray * t)
{
    int dummy;
    const size_t n = s.size();
    for (size_t i = 0; i < n; i++) {
        const void * arg = s[i].object_arg;
        object_intersection oi = s[i].intersection;
        segment p = oi(t, arg, &dummy);
        inside.assign(i, p.entry <= 0 && p.exit_ >= 0);
    }
}

    static real
intersect(const ray * t, scene_object const * so, int * hit, bool is_inside)
{
        const void * intersection_arg = so->object_arg;
        const segment p = so->intersection(t, intersection_arg, hit);
        //if (is_inside != (p.entry <= 0 && p.exit_ >= 0)) {
        //    fprintf(stderr, "inside-tracking error.  correcting\n");
        //    is_inside = ! is_inside; // inside->flip(i);
        //}
        return is_inside ? p.exit_ : p.entry;
}

    scene_object const *
closest_surface(scene const & s, ray * const t, bitarray & inside, stack * flipped)
{
    advance(t, TINY_REAL);
    scene_object const * closest_object = nullptr;
    real closest_r = -1;
    int closest_i = -1;
    const size_t n = s.size();
    std::vector<int> hits(n);
    for (size_t i = 0; i < n; i++) {
        const real r = intersect(t, &s[i], &hits[i], inside.isset(i));
        if (r >= 0 && (closest_r < 0 || r < closest_r)) {
            closest_object = &s[i];
            closest_r = r;
            closest_i = static_cast<int>(i);
        }
    }
    if (closest_r >= 0 && closest_r < HUGE_REAL) {
        int precedent_i = inside.firstset();
        if (precedent_i >= 0 && closest_i > precedent_i) {
            advance(t, closest_r);
            inside.flip(closest_i);
            closest_object = closest_surface(s, t, inside, flipped);
            if (closest_object && flipped != nullptr)
                flipped->push(closest_i);
            else
                inside.flip(closest_i);
        } else {
            advance(t, closest_r);
            const void * normal_arg = closest_object->object_arg;
            t->head = closest_object->normal(t->endpoint, normal_arg, hits[closest_i]);
        }
        return closest_object;
    } else {
        return nullptr;
    }
}
