//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef SCENE_HPP
#define SCENE_HPP

#include "scene.h"
#include <vector>

using scene = std::vector<scene_object>;
using bitarray = std::vector<bool>;
int firstset(bitarray & bits);
void init_inside(bitarray & inside, scene const &, const ray *);
scene_object const * closest_surface(scene const &, ray * const t,
    bitarray & inside);

#endif
