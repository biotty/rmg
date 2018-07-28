
#include "world_gen.hpp"

#include <cmath>

using namespace model;

world wgen(int i, int n)
{
    double seqt = i /(double) n;
    color glass = from_hsv(red_hue, .2, .9);
    point crank = xz(.6, .5);

    object tube{
        {
            sphere{o, .9},
            cylinder{o, .5, xd},
            saddle{o, zd, xd, 3},
            inv_hyperbol{o, .07, zd, .17},
            inv_parabol{onx(-.7), .4, xd},
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

    int n_trips = 8;

    point handle{
        crank + xyc(seqt * n_trips * pi2) * .1
    };

    object ball{
        {
            sphere{handle, .1},
        },
        {
            gray(.04),
            black,
            glass_ri,
            black,
            white,
        },
        mapping_f{o, 1, zd, xd, [](point,direction) -> surface {
            return {black,black,white}; }},
        /*
        checkers{
            handle, .16, zd, xyc(seqt * n_trips * pi2), 9,
            {
                white,
                black,
                black
            }
        }
        */
    };

    rotation rot{direction_cast(xy(1, 2)), seqt * pi2};
    return {
        { yz(.1, 2), o, xd * .65 },
            //      photo_sky{"sky.jpeg"},
            [](direction d) -> color { return {
                (abs(d.x) < .01) ? 0. : (d.x + 1) * 0.5,
                (abs(d.y) < .01) ? 1. : 0.,
                (abs(d.x) < .01) ? 1. : 0.}; },
        {
            tube.rot(o, rot),
            ball.rot(o, rot),
        },
        {}
    };
}

int main()
{
    sequence(wgen, 400, "", hdtv, 0);
}
