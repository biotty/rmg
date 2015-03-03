//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "color.h"

    void
filter(color * light, compact_color surface)
{
    light->r *= surface.r /(real) 255;
    light->g *= surface.g /(real) 255;
    light->b *= surface.b /(real) 255;
}

    color
optical_sum(color q, color w)
{
    q.r += w.r;
    q.g += w.g;
    q.b += w.b;
    return q;
}

    real
intensity(color c)
{
    return (c.r + c.g + c.b) / 3;
}
