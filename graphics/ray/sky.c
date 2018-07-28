//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "sky.h"
#include "photo.h"


void * sky_arg;


    color
photo_sky(direction d)
{
    real x, y;
    direction_to_unitsquare(&d, &x, &y);
    const photo_attr * a = sky_arg;
    const real col = (x == 0) ? a->width - 1 : (1 - x) * a->width;
    // ^ horizontally flip as we see the "sphere" from the "inside"
    compact_color cc = photo_color(sky_arg, col, y * a->height);
    return x_color(cc);
}

    color
rgb_sky(direction d)
{
    return (color){
        (d.x + 1) * 0.5,
        (d.y + 1) * 0.5,
        1 - (d.z + 1) * 0.5 // <-- natural orient for blue down
    };
}

    color
hsv_sky(direction d)
{
    real x, y;
    direction_to_unitsquare(&d, &x, &y);
    const real h = x * REAL_PI * 2;
    const real s = y > 0.5 ? 1 : 2 * y;
    const real v = y < 0.5 ? 1 : 2 * (1 - y);
    return from_hsv(h, s, v);
}
