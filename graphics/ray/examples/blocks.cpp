#include "rayt.hpp"
#include "sky.h"

#include <algorithm>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

using namespace rayt;
using namespace std;

double randd()
{
  return (double)rand() / ((double)RAND_MAX + 1);
}

struct wgen {
    double eye_phi;
    struct block_fixture {
        point p;
        direction v;
        double h;
    };
    vector<block_fixture> block_fixtures;

    struct block_optics : optics {
        block_optics(double hue) : optics{
            from_hsv(hue, .22, .11),
            black,
            water_ri,
            from_hsv(hue, .45, .98),
            from_hsv(hue, .6, .8) }
        {}
    };

    void make_block(vector<object> &blocks, block_fixture f, double seqt) {
        double w = 1;
        double h = w * 4. / 3;
        double b = w - h / 3;
        point c = f.p + zd * (h / 2) + f.v * (seqt - .5);
        object box {
            {
                cuboid{c, zd, xd, {w, w, h}},
                inv_cuboid{c +- zd * (h / 6), zd, xd, {b, b, h}},
            },
            block_optics(f.h), {}
        };
        blocks.push_back(box);
        object bump {
            {
                cylinder{c, b / 2, zd},
                inv_plane{c + zd * (h / 2 - TINY_REAL), zd},
                plane{c + zd * (h * 2. / 3), zd},
            },
            block_optics(f.h), {}
        };
        blocks.push_back(bump);
    }

    wgen(int n);
    world operator()(double seqt);
};

wgen::wgen(int n)
{
    eye_phi = randd() * pi * 2;
    for (int i=0; i<n; i++) {
        double x = (randd() - .5) * 9;
        double y = (randd() - .5) * 9;
        point p = o + xd * x + yd * y;
        double s = (randd() - .5) * 4;
        direction v = {0, 0, 0};
        if (randd() > .5) {
            v.x = s;
        } else {
            v.y = s;
        }
        double h = randd() * pi * 2;
        block_fixtures.push_back({ p, v, h });
    }
}

world wgen::operator()(double seqt)
{
    vector<object> blocks;
    for (auto f : block_fixtures) {
        make_block(blocks, f, seqt);
    }

    const direction right = xyc(eye_phi + pi * .5);
    return {
        observer{ o +- xyc(eye_phi) * 7+ zd*3, o, right*5 }, rgb_sky,
        blocks,
        {}
    };
}

int main(int argc, char ** argv)
{
    char *e = getenv("SRAND");
    unsigned s = e ? atoi(e) : time(NULL);
    cout << "SRAND=" << s << "\n";
    srand(s);
    args(argc, argv).run(wgen(24));
}
