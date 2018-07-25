
#include "world_gen.hpp"

using namespace model;

world wgen(int i, int n)
{
    double seqt = i /(double) n;
    color glass = from_hsv(red_hue, .2, .9);
    point crank = xz(.6, .5);

    object tube{
        {
            sphere{o, .9},
            cylinder{o, xd, .5},
            saddle{o, zd, xd, 3},
            inv_hyperbol{o, zd, .07, .17},
            inv_parabol{onx(-.7), xd, .4},
            inv_sphere{crank, .1},
        },
        {
            gray(.45),
            black,
            glass_ri,
            glass * .55,
            glass,
        },
        {}
    };

    object ball{
        {
            sphere{crank + circle(seqt * 8 * pi2) * .1, .05},
        },
        {
            gray(.04),
            black,
            0,
            black,
            black,
        },
        {}
    };

    rotation rot{direction_cast(xy(1, 2)), seqt * pi2};
    return {
        { yz(.1, 2), o, xd * .65 },
        rgb_sky,
        {
            tube.rot(o, rot),
            ball.rot(o, rot),
        },
        {}
    };
}

int main()
{
    sequence(wgen, 400);
}
