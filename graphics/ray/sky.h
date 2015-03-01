//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef SKY_H
#define SKY_H

#include "color.h"
#include "direction.h"
#include "photo.h"

typedef color (*scene_sky)(direction d);
extern photo * sky_photo;

#ifdef __cplusplus
extern "C" {
#endif

color white_sky(direction);
color photo_sky(direction);
color funky_sky(direction);

#ifdef __cplusplus
}
#endif

#endif
