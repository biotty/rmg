#ifndef PHOTO_H
#define PHOTO_H

#include "color.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Photo {
    int width;
    int height;
    bool grey;
    int ref_count_;
    char data[];
} photo;

photo * photo_create(const char * path);
unsigned photo_rgb(const photo *, int x, int y);
color photo_color(const photo *, int x, int y);
void photo_delete(photo *);

#ifdef __cplusplus
}
#endif
#endif

