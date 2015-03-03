//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef SCENE_H
#define SCENE_H

#include "color.h"
#include "ray.h"
#include "bitarray.h"
#include "stack.h"
#include "stddef.h"

typedef real_pair (* object_intersection)(const ray *, void * object_arg);
typedef direction (* object_normal)(point, void * object_arg, bool at_second);

struct object_optics;
typedef void (* object_decoration)(const ray *, void * arg,
        struct object_optics * so, const struct object_optics * adjust);
typedef void (* decoration_delete)(void *);

struct object_optics {
    unsigned refraction_index_micro;
    compact_color reflection_filter;
    compact_color absorption_filter;
    compact_color refraction_filter;
    compact_color passthrough_filter;
};

struct scene_object {
    object_intersection intersection;
    object_normal normal;
    void * object_arg;
    struct object_optics optics;
    object_decoration decoration;
    void * decoration_arg;
};

struct scene {
    int object_count;
    struct scene_object * objects;
};

#ifndef __cplusplus
typedef struct object_optics object_optics;
typedef struct scene_object scene_object;
typedef struct scene scene;
#endif

#ifdef __cplusplus
extern "C" {
#endif

void init_inside(bitarray *, scene s, const ray *);
scene_object * closest_surface(ray *, const scene s, bitarray * inside, stack * toggled);

#ifdef __cplusplus
}
#endif

#endif
