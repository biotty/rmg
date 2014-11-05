//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "sphere.h"
#include "math.h"
#include <stdio.h>
#include <assert.h>

    int
main(void)
{
    sphere sphere_;
    ray ray_;
    sphere_.center = (point){1, 1, 1};
    sphere_.radius = 1;
    ray_.endpoint = (point){0, 0, 0};
    ray_.head = (direction){1, 1, 1};
    normalize(&ray_.head);
    const real sqrt_1_3 = 0.57735;
    assert(is_pretty_near(ray_.head.x, sqrt_1_3));
    assert(is_pretty_near(ray_.head.y, sqrt_1_3));
    assert(is_pretty_near(ray_.head.z, sqrt_1_3));
    const pair p = sphere_intersection(&ray_, &sphere_);
    //printf(REAL_FMT "\n", p.second);
    assert(is_pretty_near(p.first, 0.73205));
    assert(is_pretty_near(p.second, 2.73205));
}

