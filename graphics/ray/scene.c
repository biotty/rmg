//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "scene.h"
#include <stdio.h>

    void
init_inside(bitarray * inside, scene s, const ray * ray_)
{
    for (int i = 0; i < s.object_count; i++) {
        void * arg = s.objects[i].object_arg;
        object_intersection oi = s.objects[i].intersection;
        real_pair p = oi(ray_, arg);
        ba_assign(inside, i, p.first <= 0 && p.second >= 0);
    }
}

    real
intersect(const ray * ray_, scene_object * so, bool is_inside)
{
        void * intersection_arg = so->object_arg;
        const real_pair p = so->intersection(ray_, intersection_arg);
        //if (is_inside != (p.first <= 0 && p.second >= 0)) {
        //    fprintf(stderr, "inside-tracking error.  correcting\n");
        //    is_inside = ! is_inside; //ba_toggle(inside, i);
        //}
        return ( ! is_inside)
            ? p.first
            : p.second;
}

    scene_object *
closest_surface(ray * ray_, const scene s, bitarray * inside, stack * toggled)
{
    advance(ray_, - TINY_REAL);
    scene_object * closest_object = NULL;
    real closest_r = -1;
    int closest_i = -1;
    for (int i = 0; i < s.object_count; i++) {
        const bool inside_ = ba_isset(inside, i);
        const real r = intersect(ray_, &s.objects[i], inside_);
        if (r >= 0 && (closest_r < 0 || r < closest_r)) {
            closest_object = &s.objects[i];
            closest_r = r;
            closest_i = i;
        }
    }
    if (closest_r >= 0 && closest_r < HUGE_REAL) {
        int presedent_i = ba_firstset(inside);
        if (presedent_i >= 0 && closest_i > presedent_i) {
            advance(ray_, closest_r + TINY_REAL);
            ba_toggle(inside, closest_i);
            closest_object = closest_surface(ray_, s, inside, toggled);
            if (closest_object && toggled != NULL)
                st_push(toggled, closest_i);
            else
                ba_toggle(inside, closest_i);
        } else {
            advance(ray_, closest_r);
            const bool inside_ = ba_isset(inside, closest_i);
            void * normal_arg = closest_object->object_arg;
            ray_->head = closest_object->normal(ray_->endpoint, normal_arg, inside_);
        }
        return closest_object;
    } else {
        return NULL;
    }
}
