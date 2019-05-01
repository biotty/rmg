#include "world_gen.hpp"

using namespace model;

world wgen(double seqt)
{
    color glass = from_hsv(red_hue, .2, .9);

    object tube{
        {
            sphere{o, .9},
            cylinder{o, .5, xd},
            saddle{o, zd, xd, 7},
            inv_hyperbol{o, .1, zd, .17},
            inv_parabol{onx(-.7), .4, xd},
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

    object belt{
        {},
        {
            gray(.04),
            black,
            glass_ri,
            black,
            white,
        },
        mapping_f{o, 1, zd, xd,
            [](point, direction) -> surface {
            return {black, black, white}; }
        },
    };

    rotation rot{direction_cast(xy(1, 2)), seqt * tau};

    world r{
        { xy(.1, 2), o, xd * .65 },
            [](direction d) -> color { return {
               (abs(d.x) < .01) ? 1. : 0.,
               (abs(d.y) < .01) ? 1. : 0.,
               (abs(d.x) < .01) ? 0. : (d.x + 1) * 0.5}; },
        {}, {}
    };

    int n_balls = 9;
    double n_trips = 3 /(double) n_balls;
    point beltc = onx(.61);
    for (int k=0; k<n_balls; k++) {
        double j = k /(double) n_balls;
        point a{ beltc + yzc((j + seqt * n_trips) * tau) * .5};
        point b{ beltc + yzc((j - seqt * n_trips) * tau) * .5};

        tube.si.push_back(inv_sphere{a, .1});
        r.s.push_back(object{
            { sphere{b, .08} },
            {
                gray(.04),
                black,
                glass_ri,
                black,
                white,
            },
            mapping_f{o, 1, zd, xd,
                [](point, direction) -> surface {
                    return {black, black, gray(.8)}; }
            },
        }.rot(o, rot));
    }
    r.s.push_back(tube.rot(o, rot));

    return r;
}

int main(int argc, char ** argv)
{
    main(wgen, argc, argv);
}
