//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef SKY_H
#define SKY_H

#include "direction.h"
#include "color.h"

typedef color (*scene_sky)(direction d);
extern void * sky_arg;

#ifdef __cplusplus
extern "C" {
#endif

color ph_sky(direction d);
color rgb_sky(direction d);
color hsv_sky(direction d);

#ifdef __cplusplus
}
#endif

#endif
