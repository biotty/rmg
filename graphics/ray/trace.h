//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef TRACE_H
#define TRACE_H

#include "scene.h"
#include "sky.h"

static const bool debug = false;
static const bool verbose = false;
static const int max_hops = 9;
static const bool transparent_refraction_on_equal_index = false;//true;
static const real reflection_on_inside = 1;//0.65;
static const bool eliminate_direct_sky = false;
static const color DIRECT_SKY = {1, 1, 1};

typedef struct {
    point point;
    color color;
} light_spot;

typedef void * world;
world alloc_world(int object_count);
void set_object(world, int, scene_object);
void set_sky(world, scene_sky sky);
void set_spots(world world_, light_spot * spots, int count);
void destroy_world(world);

color trace(ray t, world);

color white_sky(direction);
color funky_sky(direction);
color ocean_sky(direction);

#endif
