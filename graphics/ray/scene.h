//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef SCENE_H
#define SCENE_H

#include "color.h"
#include "ray.h"

typedef segment (* object_intersection)(const ray *, const void * object_arg, int * hit);
typedef direction (* object_normal)(point, const void * object_arg, int hit);

struct object_optics;
typedef void (* object_decoration)(const ray *, const void * arg,
        struct object_optics * so, const struct object_optics * adjust);
typedef void (* decoration_delete)(void *);

struct object_optics {
    float refraction_index;
    compact_color reflection_filter;
    compact_color absorption_filter;
    compact_color refraction_filter;
    compact_color passthrough_filter;
};

struct scene_object {
    object_intersection intersection;
    object_normal normal;
    const void * object_arg;
    struct object_optics optics;
    object_decoration decoration;
    const void * decoration_arg;
};

#ifndef __cplusplus
typedef struct object_optics object_optics;
typedef struct scene_object scene_object;
typedef struct scene scene;
#endif

#endif
