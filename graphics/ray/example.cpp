
#include "model.hpp"

#include <iostream>
#include <sstream>

using namespace model;

void movie_frame(int i, int n)
{
    double t = i /(double) n;

    object sph{
        {
            sphere{o, .9},
            cylinder{o, xd, .5},
            saddle{o, zd, xd, 3},
            inv_hyperbol{o, zd, .07, .17},
            inv_parabol{onx(-.7), xd, .4}
        },
        {
            {.4, .4, .4},
            {0, 0, 0},
            1.2,
            {.5, .5, .6},
            {.95, .97, .99}
        },
        {}
    };

    rot_(sph.si, o, {direction_cast(xy(1, 2)), t * pi2});

    world w{
        { onz(9), o, xd * .6 },
        rgb_sky,
        { sph },
        {}
    };

    std::ostringstream path("out");
    path << i << ".jpeg";
    render(path.str(), hdtv, w);
    std::cout << "\r" << i << std::flush;
}

int main()
{
    int n = 300;

    for (int i=0; i<n; i++) movie_frame(i, n);
}
