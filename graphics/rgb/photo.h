//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef PHOTO_H
#define PHOTO_H

#include "color.h"
#include <stdbool.h>

struct photo {
    int width;
    int height;
    bool grey;
    int ref_count_;
    char data[];
};

#ifndef __cplusplus
typedef struct photo photo;
#endif

#ifdef __cplusplus
extern "C" {
#endif

photo * photo_create(const char * path);
unsigned photo_rgb(const photo *, int x, int y);
compact_color photo_color(const photo *, int x, int y);
void photo_delete(photo *);

#ifdef __cplusplus
}
#endif

#endif
