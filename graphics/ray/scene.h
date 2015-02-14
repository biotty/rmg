//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef SCENE_H
#define SCENE_H

#include "color.h"
#include "ray.h"
#include "bitarray.h"
#include "stack.h"
#include "stddef.h"

typedef pair (* object_intersection)(const ray *, void * object_arg);
typedef direction (* object_normal)(point, void * object_arg, bool at_second);

struct object_optics__;
typedef void (* object_decoration)(const ray *, void * arg,
        struct object_optics__ *);
typedef void (* decoration_delete)(void *);

typedef struct object_optics__ {
    color reflection_filter;
    color absorption_filter;
    real refraction_index;
    color refraction_filter;
    color traversion_filter;
} object_optics;

typedef struct scene_object__ {
    object_intersection intersection;
    object_normal normal;
    void * object_arg;
    object_optics optics;
    object_decoration decoration;
    void * decoration_arg;
} scene_object;

typedef struct {
    int object_count;
    scene_object objects[];
} scene;

size_t scene_size(int object_count);
void init_inside(bitarray *, scene *, const ray *);
scene_object * closest_surface(ray *, scene *, bitarray * inside, stack * toggled);

#endif

