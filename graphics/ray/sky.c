//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "sky.h"
#include "photo.h"
#include "direction.h"

#include <string.h>


photo * sky_photo;
double sky_color[3];


    static direction
get(double * xyz)
{
    return (direction){
        xyz[0],
        xyz[1],
        xyz[2]};
}

    static void
set(color c, double * rgb)
{
    rgb[0] = (double)c.r;
    rgb[1] = (double)c.g;
    rgb[2] = (double)c.b;
}

    void
color_sky(double * xyz_rgb)
{
    memcpy(xyz_rgb, sky_color, sizeof sky_color);
}

    void
photo_sky(double * xyz_rgb)
{
    direction d = get(xyz_rgb);
    real x, y;
    direction_to_unitsquare(&d, &x, &y);
    const photo_attr * a = (photo_attr *) sky_photo;
    const real col = (x == 0) ? a->width - 1 : (1 - x) * a->width;
    // ^ horizontally flip as we see the "sphere" from the "inside"
    compact_color cc = photo_color(sky_photo, col, y * a->height);
    set(x_color(cc), xyz_rgb);
}

    void
rgb_sky(double * xyz_rgb)
{
    xyz_rgb[0] = (xyz_rgb[0] + 1) * 0.5;
    xyz_rgb[1] = (xyz_rgb[1] + 1) * 0.5;
    xyz_rgb[2] = 1 - (xyz_rgb[2] + 1) * 0.5; // <-- natural orient for blue down
}

    void
hsv_sky(double * xyz_rgb)
{
    direction d = get(xyz_rgb);
    real x, y;
    direction_to_unitsquare(&d, &x, &y);
    const real h = x * REAL_PI * 2;
    const real s = y > 0.5 ? 1 : 2 * y;
    const real v = y < 0.5 ? 1 : 2 * (1 - y);
    set(from_hsv(h, s, v), xyz_rgb);
}
