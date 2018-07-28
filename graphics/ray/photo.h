//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef PHOTO_H
#define PHOTO_H

#include "color.h"
#include <stdbool.h>

struct photo;
// note: first member is
struct photo_attr {
    int width;
    int height;
    bool grey;
};

#ifndef __cplusplus
typedef struct photo photo;
typedef struct photo_attr photo_attr;
#endif

#ifdef __cplusplus
extern "C" {
#endif

photo * photo_create(const char * path);
unsigned photo_rgb(const photo *, int x, int y);
compact_color photo_color(const photo *, int x, int y);
void photo_delete(photo *);
void photo_incref(photo *);

#ifdef __cplusplus
}
#endif

#endif
