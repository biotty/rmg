//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef TRACE_HPP
#define TRACE_HPP

#include "sky.h"
#include "scene.hpp"
#include <cstdlib>

constexpr bool verbose = false;

struct light_spot {
    point spot;
    color light;
};

using spots = std::vector<light_spot>;

struct world {
    typedef void (* del_f)(void *);
    scene_sky sky;
    spots spots_;
    scene scene_;
    void scene_assigned();
    std::vector<float> surface_ranks;
    world(scene_sky sky, del_f inter_f, del_f decoration_f);
    world(const world&) = delete;
    world(world&&) = default;
    world& operator=(const world&) = delete;
    world& operator=(world&&) = default;
    ~world();
    del_f del_inter;
    del_f del_decoration;
    std::vector<void *> decoration_args;
    std::vector<void *> object_args;
    std::vector<void *> inter_args;
};

template <typename T>
T * alloc()
{
    return static_cast<T *>(malloc(sizeof (T)));
}

color trace(ray t, const world &);

#endif
