//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef SKY_H
#define SKY_H

typedef void (*scene_sky)(double *xyz_rgb);

#ifdef __cplusplus
extern "C" {
#endif

void color_sky(double *);
void photo_sky(double *);
void rgb_sky(double *);
void hsv_sky(double *);
extern struct photo * sky_photo;
extern double sky_color[3];

#ifdef __cplusplus
}
#endif

#endif
