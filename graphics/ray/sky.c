//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "sky.h"
#include "photo.h"


photo * sky_photo;
color sky_color;


    color
color_sky(direction d)
{
    (void)d;
    return sky_color;
}

    color
photo_sky(direction d)
{
    real x, y;
    direction_to_unitsquare(&d, &x, &y);
    const photo_attr * a = (photo_attr *) sky_photo;
    const real col = (x == 0) ? a->width - 1 : (1 - x) * a->width;
    // ^ horizontally flip as we see the "sphere" from the "inside"
    compact_color cc = photo_color(sky_photo, col, y * a->height);
    return x_color(cc);
}

    color
rgb_sky(direction d)
{
    return (color){
        (d.x + 1) * 0.5,
        (d.y + 1) * 0.5,
    1 - (d.z + 1) * 0.5  // most natural orientation for blue is down
    };
}
