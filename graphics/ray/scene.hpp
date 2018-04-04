//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef SCENE_HPP
#define SCENE_HPP

#include "scene.h"
#include "bitarray.hpp"
#include "stack.hpp"

void init_inside(bitarray & inside, scene, const ray *);
scene_object * closest_surface(scene, ray * const t,
    bitarray & inside, stack * flipped);

#endif
