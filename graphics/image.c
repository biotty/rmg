
#include "image.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <stdbool.h>

    static bool //note: exist in photo.c as well
path_is_jpeg(const char * path)
{
    size_t n = strlen(path);
    if (n >= 4 && 0 == strcasecmp(path + n - 4, ".jpg")) return true;
    if (n >= 5 && 0 == strcasecmp(path + n - 5, ".jpeg")) return true;
    return false;
}

    image
image_create(const char * path, int x, int y)
{
    extern FILE * popen(const char *, const char *);
    char header[32];
    const int n = sprintf(header, "P6\n%d %d 255\n", x, y);
    FILE * file = stdout;
    if (path) {
        if (path_is_jpeg(path)) {
            char cmd[256];
            snprintf(cmd, sizeof cmd, "pnmtojpeg > %s", path);
            file = popen(cmd, "w");
        } else
            file = fopen(path, "wb+");
        if ( ! file) return NULL;
    }
    int c = fwrite(header, 1, n, file);
    if (c != n) {
        if (path) fclose(file);
        return NULL;
    } else {
        return file;
    }
}

    static inline void
intensity_cap(color * color_)
{
    // alternative: could distribute exess
    if (color_->r > 1) color_->r = 1;
    if (color_->g > 1) color_->g = 1;
    if (color_->b > 1) color_->b = 1;
}

    void
image_write(image image_, color color_)
{
    FILE * file = image_;
    intensity_cap(&color_);
    putc(nearest(color_.r * 255), file);
    putc(nearest(color_.g * 255), file);
    putc(nearest(color_.b * 255), file);
}

    void
image_close(image image_)
{
    FILE * file = image_;
    fclose(file);
}

