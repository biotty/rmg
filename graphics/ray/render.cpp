//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "render.hpp"
#include "work.hpp"
#include "image.h"

#include <iostream>
#include <algorithm>
#include <utility>
#include <cstdlib>


    ray
observer_ray(const observer & o, real aspect_ratio,
        real unit_column, real unit_row)
{
    direction x = o.column_direction;
    direction y = o.row_direction;
    scale(&x, aspect_ratio * (unit_column - .5));
    scale(&y, unit_row - .5);
    point v = o.view;
    move(&v, x);
    move(&v, y);
    ray ray_{v, distance_vector(o.eye, v)};
    move(&ray_.endpoint, distance_vector(o.view, o.eye));
    normalize(&ray_.head);
    return ray_;
}


    void
render(const char * path, int width, int height,
        const observer & obs, const world & w, unsigned n_threads)
{
    if (n_threads == 0)
        n_threads = std::thread::hardware_concurrency();

    image out = image_create(path, width, height);
    if ( ! out) {
        std::cerr << "ray: cannot create image " << path << "\n";
        exit(EXIT_FAILURE);
    }
    if (n_threads <= 1) {
        for (int y=0; y!=height; ++y)
            for (int x=0; x!=width; ++x)
                image_write(out,
                       trace(observer_ray(obs, width /(real) height,
                            (x + (real).5) / width,
                            (y + (real).5) / height), w));
    } else {
        work<color> q{n_threads, 16, width * height,
            [width, height, &obs, &w](int seq_i) {
                div_t d = div(seq_i, width);
                return trace(observer_ray(obs, width /(real) height,
                            (d.rem + (real).5) / width,
                            (d.quot + (real).5) / height), w);
            }
        };
        while (q.more()) {
            image_write(out, q.get_result());
        }
    }
    image_close(out);
}


    void // linkage: "C"
direct_row(observer * o)
{
    real s = length(o->column_direction);
    direction e = distance_vector(o->eye, o->view);
    direction d = cross(o->column_direction, e);
    normalize(&d);
    scale(&d, s);
    o->row_direction = d;
    d = cross(e, d);
    normalize(&d);
    scale(&d, s);
    o->column_direction = d;
}
