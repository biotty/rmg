//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "scene.h"
#include "trace.hpp"
#include <vector>
#include <algorithm>

    void
init_inside(bitarray & inside, scene const & s, const ray * t)
{
    int dummy;
    const size_t n = s.size();
    for (size_t i = 0; i < n; i++) {
        const void * arg = s[i].object_arg;
        object_intersection oi = s[i].intersection;
        segment p = oi(t, arg, &dummy);
        inside[i] = (p.entry <= 0 && p.exit_ >= 0);
    }
}

    int
firstset(bitarray & bits)
{
    auto it = std::find(bits.begin(), bits.end(), true);
    if (bits.end() == it) return -1;

    return std::distance(bits.begin(), it);
}

    static real
intersect(const ray * t, scene_object const * so, int * hit, bool is_inside)
{
        const void * intersection_arg = so->object_arg;
        const segment p = so->intersection(t, intersection_arg, hit);
        //if (is_inside != (p.entry <= 0 && p.exit_ >= 0)) {
        //    fprintf(stderr, "inside-tracking error.  correcting\n");
        //    is_inside = ! is_inside; // inside[i].flip();
        //}
        return is_inside ? p.exit_ : p.entry;
}

    scene_object const *
closest_surface(scene const & s, ray * const t, bitarray & inside)
{
    advance(t, TINY_REAL);
    scene_object const * closest_object = nullptr;
    real closest_r = -1;
    int closest_i = -1;
    const size_t n = s.size();
    std::vector<int> hits(n);
    for (size_t i = 0; i < n; i++) {
        const real r = intersect(t, &s[i], &hits[i], inside[i]);
        if (r >= 0 && (closest_r < 0 || r < closest_r)) {
            closest_object = &s[i];
            closest_r = r;
            closest_i = static_cast<int>(i);
        }
    }
    if (closest_r >= 0 && closest_r < HUGE_REAL) {
        int precedent_i = firstset(inside);
        if (precedent_i >= 0 && closest_i > precedent_i) {
            advance(t, closest_r);
            inside[closest_i].flip();
            closest_object = closest_surface(s, t, inside);
            if (nullptr == closest_object) {
                inside[closest_i].flip();
            }
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
