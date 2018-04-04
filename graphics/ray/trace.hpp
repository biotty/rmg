//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef TRACE_HPP
#define TRACE_HPP

#include "sky.h"
#include "scene.hpp"

static const bool debug = false;
static const bool verbose = false;
static const int max_hops = 11;
static const bool transparent_on_equal_index = true;
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

    world(scene_sky s, size_t q)
        : sky(s)
        , spots()
        , spot_count()
        , scene_(q)
    {}
};

color trace(ray t, world *);

#endif
