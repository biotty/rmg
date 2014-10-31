
#include "observer.h"

    ray
observer_ray(const observer * o, int column, int row)
{
    const real ratio = o->width /(real) o->height;
    direction x = o->column_direction;
    direction y = o->row_direction;
    scale(&x, ratio * (column - o->width/2) / (real)o->width);
    scale(&y, (row - o->height/2) / (real)o->height);
    point v = o->view;
    move(&v, x);
    move(&v, y);
    ray ray_ = { .endpoint = v,
        .head = distance_vector(o->eye, v)
    };
    move(&ray_.endpoint, distance_vector(o->view, o->eye));
    normalize(&ray_.head);
    return ray_;
}

