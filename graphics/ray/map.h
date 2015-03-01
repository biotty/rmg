//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef MAP_H
#define MAP_H

#include "scene.h"

struct map_application {
    real x_wrap;
    real y_wrap;
    color reflection_factor;
    color absorption_factor;
    color refraction_factor;
};

struct n_map_setup {
    direction n;
    const char * path;
    struct map_application a;
};

struct n_o_map_setup {
    direction n;
    point o;
    const char * path;
    struct map_application a;
};

#ifndef __cplusplus
typedef struct map_application map_application;
typedef struct n_map_setup n_map_setup;
typedef struct n_o_map_setup n_o_map_setup;
#endif

#ifdef __cplusplus
extern "C" {
#endif

void * map_decoration(
        object_decoration * df, const n_map_setup * setup);
void * pmap_decoration(
        object_decoration * df, const n_map_setup * setup);

void * omap_decoration(
        object_decoration * df, const n_o_map_setup * setup);
void * lmap_decoration(
        object_decoration * df, const n_o_map_setup * setup);

void generic_map_delete(void * decoration_arg);

#ifdef __cplusplus
}
#endif

#endif
