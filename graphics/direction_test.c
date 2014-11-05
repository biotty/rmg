//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "direction.h"
#include "math.h"
#include <stdio.h>
#include <assert.h>

    int
main(void)
{
    direction d = {3, 1, 2};
    normalize(&d);
    //printf(REAL_FMT "\n", d.x);
    assert(is_pretty_near(d.x, 0.802));
    assert(is_pretty_near(d.y, 0.267));
    assert(is_pretty_near(d.z, 0.534));
    direction u = {1, 1, 1};
    normalize(&u);
    direction r = reflection(d, u);
    assert(is_pretty_near(r.x, -0.907));
    assert(is_pretty_near(r.y, 0.082));
    assert(is_pretty_near(r.z, -0.412));
}

