//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "photo.h"
#include "image.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <map>

#include <unistd.h>


constexpr static const double ALL = 3;


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
static void do_fillins(int x, int y, color & c, bool skip_first = false);
static void do_gradient(int x, int y, color & c);

    int
main(int argc, char **argv)
{

    if (argc <= 1) {
        std::cerr << "Usage:\tProvide a maskable* photo file.\n\t* "
                "some (~) monochrome areas to be used to fill-in photo(s).\n"
                "\t  or a use slash to v-gradient itself or the following fill-in(s)\n"
                "Alt.usage:\tProvide no fill-ins to list colors in file\n";
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
    const photo_attr & a = *(photo_attr *)orig;
    image i = image_create(NULL, a.width, a.height);
    for (int y=0; y<a.height; y++) {
        for (int x=0; x<a.width; x++) {
            color c = x_color(photo_color(orig, x, y));
            if (fillins[0].esq < 0) do_gradient(x, y, c);
            else do_fillins(x, y, c);
            // ^^ todo: only when only one fillin, handle when two too (rest-gradient replace)
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
            std::cerr << i << ": not [color[~e]|/]:[path|=color]\n";
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
        if (*a == '/') {
            if (a[1] != ':') {
                std::cerr << i << ": trailer after slash\n";
                goto err;
            }
            f.esq = -1;
        } else if (*a == '+') {
            f.esq = ALL;
        } else {
            f.c = parse_color(a);
        }

        if (*name == '=') {
            f.r = parse_color(name + 1);
        } else {
            f.p = photo_create(name);
        }
    }
    return true;
err:return false;
}

    static void
stats()
{
    const photo_attr & a = *(photo_attr *)orig;
    std::map<unsigned, unsigned> counts;
    for (int y=0; y<a.height; y++) {
        for (int x=0; x<a.width; x++) {
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

    static int
scale(int value, int from, int to)
{
    if (from == to) return value;

    return value * (to /(double) from);
}

    static color
photo_inter(photo * p, int orig_x, int orig_y)
{
    const photo_attr & oa = *(photo_attr *)orig;
    const photo_attr & pa = *(photo_attr *)p;
    const int x = scale(orig_x, oa.width, pa.width);
    const int y = scale(orig_y, oa.height, pa.height);
    return x_color(photo_color(p, x, y));
}

    static void
do_fillins(int x, int y, color & c, bool skip_first)
{
    for (const auto & f : fillins) {
        if (skip_first) skip_first = false;
        if (f.esq >= ALL || similar(f.esq, &c, &f.c)) {
            c = f.p ? photo_inter(f.p, x, y) : f.r;
            break;
        }
    }
}

    static void
do_gradient(int x, int y, color & c)
{
    const double rgb[] = { c.r, c.g, c.b };
    double v = *std::max_element(std::begin(rgb), std::end(rgb));
    if (fillins.size() > 1) {
        do_fillins(x, y, c, true);
        c.r *= v;
        c.g *= v;
        c.b *= v;
    }
    const auto & f = fillins[0];
    const color d = f.p ? photo_inter(f.p, x, y) : f.r;
    v = 1 - v;
    c.r += d.r * v;
    c.g += d.g * v;
    c.b += d.b * v;
}
