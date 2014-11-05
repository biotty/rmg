//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "observer.h"
#include "math.h"
#include <stdio.h>
#include <assert.h>

    int
main(void)
{
    observer observer = {
        .eye = {0, 0, -9},
        .view = {0, 0, 0},
        .column_direction = {1, 0, 0},
        .row_direction = {0, 1, 0},
        .width = 100,
        .height = 100,
    };
    ray upperleft = observer_ray(&observer, 0, 0);
    assert(is_pretty_near(upperleft.endpoint.x, -0.5));
    assert(is_pretty_near(upperleft.endpoint.y, -0.5));
    assert(is_pretty_near(upperleft.endpoint.z, -9));
    assert(is_pretty_near(upperleft.head.x, -0.0554));
    assert(is_pretty_near(upperleft.head.y, -0.0554));
    assert(is_pretty_near(upperleft.head.z, 0.9969));
}

