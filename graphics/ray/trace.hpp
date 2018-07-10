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

using spots = std::vector<light_spot>;

struct world {
    scene_sky sky;
    spots spots_;
    scene scene_;
    world() : sky() {}
};

color trace(ray t, const world &);

#endif
