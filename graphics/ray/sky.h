//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef SKY_H
#define SKY_H

#include "color.h"
#include "direction.h"
#include "photo.h"

typedef color (*scene_sky)(direction d);

#ifdef __cplusplus
extern "C" {
#endif

color color_sky(direction);
color photo_sky(direction);
color rgb_sky(direction);
color hsv_sky(direction);
extern photo * sky_photo;
extern color sky_color;

#ifdef __cplusplus
}
#endif

#endif
