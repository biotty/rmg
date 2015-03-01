//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef TRACE_H
#define TRACE_H

#include "scene.h"
#include "sky.h"

static const bool debug = false;
static const bool verbose = false;
static const int max_hops = 6;
static const bool transparent_refraction_on_equal_index = false;
static const real reflection_on_inside = .9;
static const bool eliminate_direct_sky = false;
static const color DIRECT_SKY = {.8, .8, .8};

struct light_spot {
    point spot;
    color light;
};

struct world {
    scene_sky sky;
    struct light_spot * spots;
    int spot_count;
    scene scene_;
};

#ifndef __cplusplus
typedef struct light_spot light_spot;
typedef struct world world;
#endif

#ifdef __cplusplus
extern "C" {
#endif

color trace(ray t, world *);

#ifdef __cplusplus
}
#endif

#endif
