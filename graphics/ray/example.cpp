
#include "model.hpp"

using namespace model;

int main()
{
    object sph{
        {
            sphere{o, .9},
            saddle{o, zd, xd, 3},
            inv_hyperbol{o, zd, .07, .25},
            inv_parabol{onx(-.6), xd, .7}
        },
        {
            {.5, .5, .5},
            {0, 0, 0},
            1.2,
            {.5, .5, .6},
            {.8, .9, .9}
        },
    };
    rot_(sph.si, o, {direction_cast(xy(1, 2)), -.3});

    observer obs{
        onz(9), o, xd
    };

    world w{
        obs,
        hsv_sky,
        { sph },
    };

    render("out.jpeg", 1920, 1080, w);
}
