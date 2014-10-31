
#include "color.h"

    void
filter(color * light, color surface)
{
    light->r *= surface.r;
    light->g *= surface.g;
    light->b *= surface.b;
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

