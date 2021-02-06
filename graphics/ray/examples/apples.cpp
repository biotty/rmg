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

constexpr double tiny_d = 1e2 * TINY_REAL;

double randd()
{
  return (double)rand() / ((double)RAND_MAX + 1);
}

direction sphere_random()
{
    direction r = zd;
    polar p = sphere_uniform(randd(), randd());
    rot_(r, { xyc(p.phi), p.theta });
    return r;
}

struct wgen {
    double eye_phi;
    struct apple_fixture {
        point p;
        double r;
        double a;
        rotation pr;
        double h;
        struct bite { double a, b; };
        vector<bite> bites;
        direction v;
        double av;
    };
    vector<apple_fixture> apple_fixtures;

    struct apple_optics : optics {
        apple_optics(double hue) : optics{
            from_hsv(hue, .7, .1),
            black,
            glass_ri,
            from_hsv(hue, .4, .8),
            white}
        {}
    };

    wgen(int n);
    world operator()(double seqt);
};

wgen::wgen(int n)
{
    eye_phi = 1;
    for (int i=0; i<n; i++) {
        point p = o + sphere_random() * 5 * randd();
        double r = (randd() + 1) * .5;
        double a = randd() * 2 * pi;
        rotation pr = { sphere_random(), randd() * pi };
        double h = randd() * pi * 1. / 3;
        if (h > 1. / 6) h += pi * 1. / 3;
        direction v = sphere_random() * 5 * randd();
        double av = randd() * 4;
        apple_fixture f = { p, r, a, pr, h, {}, v, av };
        for (int j=0; j<17; j++) {
            f.bites.push_back({ randd(), randd() });
        }
        apple_fixtures.push_back(f);
    }
}

world wgen::operator()(double seqt)
{
    vector<object> objects;
    for (auto f : apple_fixtures) {
        const auto opts = apple_optics(f.h);
        const point ca = f.p + f.v * (seqt - .5);
        const double h = f.r * .4;
        direction ad = xyc(f.a);
        direction bd = xyc(f.a + pi * .5);
        direction cd = zd;
        rot_(ad, f.pr);
        rot_(bd, f.pr);
        rot_(cd, f.pr);

        {
            const double a = seqt * f.av;
            rot_(ad, { cd, a });
            rot_(bd, { cd, a });
        }

        inter apple = {
            sphere{ca, f.r},
            hyperbol{ca, h, ad, f.r * .5},
        };
        for (auto bite : f.bites) {
            direction bbd = bd;
            rot_(bbd, { ad, bite.a * 2 * pi });
            apple.push_back(inv_sphere{
                ca + bbd * (h + f.r * .95) + ad * ((bite.b - .5) * 2 * f.r),
                f.r * .95});
        }
        auto apple_obj = object{ apple, opts, {} };

        inter trunk = {
            cone{ca + ad * (f.r - .1), 5, ad},
            plane{ca + ad * (f.r + .1), ad},
            inv_plane{ca + ad * (f.r - tiny_d), ad},
        };
        auto trunk_obj = object{ trunk, opts, {} };

        inter leaf = {
            plane{ca + ad * (f.r + .036), ad},
            sphere{ca + ad * f.r * 2 + bd * .19, f.r * .98},
        };
        auto leaf_obj = object{ leaf, opts, {} };

        objects.push_back(apple_obj);
        objects.push_back(trunk_obj);
        objects.push_back(leaf_obj);
    }

    const direction right = xyc(eye_phi + pi * .5);
    return {
        observer{ o + zd * 3 +- xyc(eye_phi) * 5, o, right * 2 },
            [](direction d) -> color {
                if (d.z > .5) {
                    return white * 2.1;
                }
                if (abs(d.x) < .01 * pi) {
                    if (d.x < 0) {
                        return {1., .45, .25};
                    }
                    return black;
                }
                if (abs(d.y) < .02 * pi) {
                    const int i = atan2(d.z, d.x) * 50 / pi;
                    if (1 & (i ^ int(d.y < 0))) return blue;
                }
                if (abs(d.z) < .02 * pi && (d.x > 0) == (d.y > 0)) {
                    const int i = atan2(d.y, d.x) * 50 / pi;
                    return (1 & (i ^ int(d.z < 0))) ? red : white;
                }
                return white * .5 * (d.z + 1);
            },
        objects,
        {}
    };
}

int main(int argc, char ** argv)
{
    char *e = getenv("SRAND");
    unsigned s = e ? atoi(e) : time(NULL);
    cout << "SRAND=" << s << "\n";
    srand(s);
    args(argc, argv).run(wgen(48));
}
