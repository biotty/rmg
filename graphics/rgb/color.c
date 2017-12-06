//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "color.h"
#include <stdlib.h>

    void
filter(color * light, compact_color surface)
{
    light->r *= surface.r /(real) 255;
    light->g *= surface.g /(real) 255;
    light->b *= surface.b /(real) 255;
}

    void
color_add(color * q, color w)
{
    q->r += w.r;
    q->g += w.g;
    q->b += w.b;
}

    real
intensity(color c)
{
    return .299 * c.r + .587 * c.g + .114 * c.b;
}

    color
from_hsv(real h, const real s, const real v)
{
    if (s == 0) {
        return (color){v, v, v};
    }
    h = rmod(h, 2 * REAL_PI);
    h *= 3 / REAL_PI;
    int i = h;
    real f = h - i;
    real p = v * (1 - s);
    real q = v * (1 - s * f);
    real t = v * (1 - s * (1 - f));
    switch (i) {
    default: abort();
    case 0: return (color){v, t, p};
    case 1: return (color){q, v, p};
    case 2: return (color){p, v, t};
    case 3: return (color){p, q, v};
    case 4: return (color){t, p, v};
    case 5: return (color){v, p, q};
    }
}
