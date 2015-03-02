//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "observer.h"

    ray
observer_ray(const observer * o, real aspect_ratio,
        real unit_column, real unit_row)
{
    direction x = o->column_direction;
    direction y = o->row_direction;
    scale(&x, aspect_ratio * (unit_column - .5));
    scale(&y, unit_row - .5);
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
