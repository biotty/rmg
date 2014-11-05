//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "cone.h"
#include "math.h"
#include <stdio.h>
#include <assert.h>

#define FAR_AWAY (HUGE_REAL / 2)

    int
main(void)
{
    cone cone_;
    ray ray_;
    cone_.translate = (direction){0, 0, 0};
    cone_.theta = 0;
    cone_.phi = 0;
    cone_.r = 2;
    ray_.endpoint = (point){1, 0, 1};
    ray_.head = (direction){0, 0, -1};
    const pair i = cone_intersection(&ray_, &cone_);
    //printf(REAL_FMT " " REAL_FMT "\n", i.first, i.second);
    assert(is_pretty_near(1.5, i.first));
    assert(is_pretty_near(0.5, i.second));
    const real t = (i.first >= 0) ? i.first : i.second;
    advance(&ray_, t);
    const point p = ray_.endpoint;
    assert(is_pretty_near(1.0, p.x));
    assert(is_pretty_near(0.0, p.y));
    assert(is_pretty_near(-.5, p.z));
    direction n = cone_normal(p, &cone_, false);
    assert(is_pretty_near(0.447214, n.x));
    assert(is_pretty_near(0.000000, n.y));
    assert(is_pretty_near(0.894427, n.z));
}

