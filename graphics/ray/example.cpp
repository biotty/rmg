
#include "model.hpp"

using namespace model;

point operator*(point p, double factor)
{
    mul_(p, factor);
    return p;
}

direction operator-(point to, point from)
{
    return {
        to.x - from.x,
        to.y - from.y,
        to.z - from.z};
}

int main()
{
point x1{1, 0, 0};
point y1{0, 1, 0};
point z1{0, 0, 1};
direction dx = {1, 0, 0};

    object sph{
        inter{
            sphere{origo, .9},
            inv_sphere{point{.6, .2, .4}, .7}
        },
        optics{
            {.7, .7, .7},
            {0, 0, 0},
            1.2,
            {.5, .6, .7},
            {.6, .9, .9}
        },
    };

    observer obs{
        z1 * 9,
        origo,
        dx
    };

    world w{
        obs,
        hsv_sky,
        { sph },
        {}
    };

    render("out.jpeg", 1920, 1080, w);
}
