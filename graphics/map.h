#ifndef MAP_H
#define MAP_H

#include "scene.h"

typedef struct {
    real x_wrap;
    real y_wrap;
    color reflection_factor;
    color reflection_adjust;
    color absorption_factor;
    color absorption_adjust;
    real refraction_index;
    color refraction_factor;
    color refraction_adjust;
    color traversion_factor;
    color traversion_adjust;
} map_application;

typedef struct {
    direction n;
    const char * path;
    map_application a;
} n_map_setup;

void * map_decoration(
        object_decoration * df, const n_map_setup * setup);
void * pmap_decoration(
        object_decoration * df, const n_map_setup * setup);

typedef struct {
    direction n;
    point o;
    const char * path;
    map_application a;
} n_o_map_setup;

void * omap_decoration(
        object_decoration * df, const n_o_map_setup * setup);
void * lmap_decoration(
        object_decoration * df, const n_o_map_setup * setup);

void generic_map_delete(void * decoration_arg);

#endif

