//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "photo.h"
#include "image.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>


#define MAX_FILLINS 2

photo * orig;

struct {
    color c;
    photo * p;
} fillins[MAX_FILLINS];

static int init(int argc, char **argv);

    int
main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Provide a maskable* photo file.\n* "
                "some monochrome areas; to be used to fill-in photo(s)\n");
        return 1;
    }
    if (argc == 2) {
        fprintf(stderr, "Provide a color* and a photo to fill-in\n* "
                "specified as six hexadecimals for rgb\n");
        return 1;
    }
    orig = photo_create(argv[1]);
    if (orig == NULL) return 1;

    int n = init(argc, argv);
    if (n == 0) return 1;

    if (isatty(1)) {
        fprintf(stderr, "Redirect stdout, i.e;\n | pnmtojpeg >mix.jpeg\n");
        return 1;
    }
    image * i = image_create(NULL, orig->width, orig->height);
    for (int y=0; y<orig->height; y++) {
        for (int x=0; x<orig->width; x++) {
            compact_color cc = photo_color(orig, x, y);
            color c = x_color(cc);
            for (int k=0; k<n; k++)
                if (similar(.065, &c, &fillins[k].c)) {
                    cc = photo_color(fillins[k].p, x, y);
                    c = x_color(cc);
                    break;
                }
            image_write(i, c);
        }
    }
    image_close(i);
    return 0;
}

    static inline color
parse_color(char * hex)
{
    char * endptr;
    unsigned rgb = strtol(hex, &endptr, 16);
    if (endptr - hex != 6) {
        fprintf(stderr, "Invalid pre-colon token in \"%s\","
                " i.e FF0000 for red\n", hex);
        exit(1);
    }
    return (color){
            (255 & (rgb)) / 255.0,
            (255 & (rgb >> 8)) / 255.0,
            (255 & (rgb >> 16)) / 255.0
    };
}

    static int
init(int argc, char **argv)
{
    int j = 0;
    photo * p;
    for (int i=2; i<argc; i++) {
        if (j == MAX_FILLINS) {
            fprintf(stderr, "Max capacity %d fillins.  Skipping rest.\n", j);
            break;
        }
        char * name = strchr(argv[i], ':');
        if (name == NULL) {
            fprintf(stderr, "Argument #%d is not of format color:path\n", i);
            return 0;
        }
        ++name;
        fillins[j].c = parse_color(argv[i]);
        fillins[j].p = p = photo_create(name);
        if (p == NULL) continue;
        ++j;
        if (p->width != orig->width || p->height != orig->height) {
            fprintf(stderr, "%s is %dx%d while original is %dx%d\n",
                    name, p->width, p->height, orig->width, orig->height);
        }
    }
    return j;
}
