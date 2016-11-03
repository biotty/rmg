//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "photo.h"
#include "image.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>

#include <unistd.h>


struct fillin {
    color c;
    double esq;
    photo * p;
    color r;
    fillin() : c(), esq(), p(), r() {}
};

static photo * orig;
static std::vector<fillin> fillins;

static bool init(int argc, char **argv);
static void stats();

    int
main(int argc, char **argv)
{

    if (argc <= 1) {
        std::cerr << "Provide a maskable* photo file.\n* "
                "some monochrome areas; to be used to fill-in photo(s).\n"
                "Provide no fill-ins to list colors in file\n";
        return 1;
    }
    orig = photo_create(argv[1]);
    if (orig == NULL)
        return 1;

    if (argc == 2) {
        stats();
        return 0;
    }

    if ( ! init(argc, argv)) return 1;
    if (isatty(1)) {
        std::cerr << "Redirect stdout, i.e;\n | pnmtojpeg >mix.jpeg\n";
        return 1;
    }
    image i = image_create(NULL, orig->width, orig->height);
    for (int y=0; y<orig->height; y++) {
        for (int x=0; x<orig->width; x++) {
            compact_color cc = photo_color(orig, x, y);
            color c = x_color(cc);
            for (const auto & f : fillins) {
                if (similar(f.esq, &c, &f.c)) {
                    if (f.p) {
                        cc = photo_color(f.p, x, y);
                        c = x_color(cc);
                    } else {
                        c = f.r;
                    }
                    break;
                }
            }
            image_write(i, c);
        }
    }
    image_close(i);
    return 0;
}

    static inline color
parse_color(const char * hex)
{
    char * endptr;
    unsigned rgb = strtol(hex, &endptr, 16);
    if (endptr - hex != 6) {
        std::cerr << "Invalid pre-colon token in " << hex
                << ", must be i.e FF0000 for red\n";
        exit(1);
    }
    return { (255 & (rgb >> 16)) / 255.0,
            (255 & (rgb >> 8)) / 255.0, (255 & (rgb)) / 255.0 };
}

    static bool
init(int argc, char **argv)
{
    for (int i=2; i<argc; i++) {
        char * const a = argv[i];
        fillins.push_back(fillin());
        fillin & f = fillins.back();
        const char * name = strchr(a, ':');
        if (name == NULL) {
            std::cerr << i << ": not color[~e]:[path|=color]\n";
            goto err;
        }
        ++name;
        char * tilde = strchr(a, '~');
        if (tilde && tilde < name) {
            double e;
            if (1 != sscanf(tilde + 1, "%lf", &e)) {
                std::cerr << i << ": non-float after tilde\n";
                goto err;
            }
            f.esq = e * e;
        }
        f.c = parse_color(a);
        if (*name == '=') {
            f.r = parse_color(name + 1);
        } else {
            const photo * p = f.p = photo_create(name);
            if ( ! p) goto err;
            if (p->width != orig->width || p->height != orig->height) {
                std::cerr << name << " is " << p->width << "x" << p->height
                    << " while original is " << orig->width << "x" << orig->height << "\n";
                goto err;
            }
        }
    }
    return true;
err:return false;
}

    static void
stats()
{
    std::map<unsigned, unsigned> counts;
    for (int y=0; y<orig->height; y++) {
        for (int x=0; x<orig->width; x++) {
            compact_color cc = photo_color(orig, x, y);
            unsigned c = cc.r + (cc.g << 8) + (cc.b << 16);
            ++counts[c];
        }
    }
    std::cout << std::setfill('0') << std::hex;
    for (const auto & pair : counts) {
        const unsigned rgb = pair.first;
        std::cout
            << std::setw(6) << pair.second << "\t"
            << std::setw(2) << (rgb >> 16)
            << std::setw(2) << (255 & (rgb >> 8))
            << std::setw(2) << (255 & rgb) << "\n";

    }
}
