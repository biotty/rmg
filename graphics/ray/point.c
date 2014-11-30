//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "point.h"
#include "xmath.h"

real
distance(
        point a,
        point b)
{
    return sqroot(
            square(a.x - b.x)
            + square(a.y - b.y)
            + square(a.z - b.z));
}

real
distance_to_origo(
        point p)
{
    return sqrt(
            square(p.x)
            + square(p.y)
            + square(p.z));
}

