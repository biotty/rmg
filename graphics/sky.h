#ifndef SKY_H
#define SKY_H

#include "color.h"
#include "direction.h"
#include "photo.h"

typedef color (*scene_sky)(direction d);
extern photo * sky_photo;

color white_sky(direction);
color photo_sky(direction);
color funky_sky(direction);

#endif

