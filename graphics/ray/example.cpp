
#include "world_gen.hpp"

#include <cmath>

using namespace model;

void sky(double p[3])
{
    double y = p[1];
    hsv_sky(p);
    if (y < 0 && fmod(-y, .08) < .01) {
        double g = p[1];
        p[1] = p[2];
        p[2] = g;
    }
}

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
            sphere{handle, .05},
        },
        {
            gray(.04),
            black,
            0,
            black,
            black,
        },
        checkers{
            handle, .16, zd, xyc(seqt * n_trips * pi2), 9,
            {
                white,
                black,
                black
            }
        }
    };

    rotation rot{direction_cast(xy(1, 2)), seqt * pi2};
    return {
        { yz(.1, -2), o, xd * .65 },
        sky,
        {
            tube.rot(o, rot),
            ball.rot(o, rot),
        },
        {}
    };
}

int main()
{
    sequence(wgen, 400, "", hdtv, 1);
}
