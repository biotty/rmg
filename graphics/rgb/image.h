//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef IMAGE_H
#define IMAGE_H

#include "color.h"

typedef void * image;

#ifdef __cplusplus
extern "C" {
#endif

image image_create(const char * name, int x, int y);
void image_write(image, color);
void image_close(image);

#ifdef __cplusplus
}
#endif

#endif
