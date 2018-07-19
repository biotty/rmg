//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef SKY_H
#define SKY_H

#include "color.h"
#include "direction.h"
#include "photo.h"

typedef void (*scene_sky)(double *xyz_rgb);

#ifdef __cplusplus
extern "C" {
#endif

void color_sky(double *);
void photo_sky(double *);
void rgb_sky(double *);
void hsv_sky(double *);
extern photo * sky_photo;
extern color sky_color;

#ifdef __cplusplus
}
#endif

#endif
