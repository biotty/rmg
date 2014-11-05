//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "sky.h"
#include "photo.h"


photo * sky_photo = 0;


    color
white_sky(direction d)
{
    return (color){1, 1, 1};
}

    color
photo_sky(direction d)
{
    real x, y;
    direction_to_unitsquare(&d, &x, &y);
    real col = (x == 0) ? sky_photo->width - 1 : (1 - x) * sky_photo->width;
    //horiz-flip as we see the "sphere" from the "inside"
    return photo_color(sky_photo, col, y * sky_photo->height);
}

    color
funky_sky(direction d)
{
    return (color){
        (d.x + 1) * 0.5,
        (d.y + 1) * 0.5,
    1 - (d.z + 1) * 0.5  //blue is nice background when looking down at xy-plane
    };
}
