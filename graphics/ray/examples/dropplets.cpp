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

direction sphere_random()
{
    direction r = zd;
    polar p = sphere_uniform(randd(), randd());
    rot_(r, { xyc(p.phi), p.theta });
    return r;
}

struct wgen {
    double eye_at_face;
    double eye_phi;
    struct droplet_fixture {
        point p;
        double r;
        direction v;
        double h;
        bool is_bullet;
    };
    vector<droplet_fixture> droplet_fixtures;

    optics bullet_optics(double hue) {
        return {
         /* white * .8, */ from_hsv(hue, .1, .9),
            black,
            0,
            black,
            black
        };
    };

    optics droplet_optics(double hue) {
        return {
         /* white * .1, */ from_hsv(hue, .1, .03),
            black,
            water_ri,
         /* white * .9, */ from_hsv(hue, .25, .9),
         /* white * .8 */ from_hsv(hue, .3, .95)
        };
    };

    wgen(int n, double eye_at_face);
    world operator()(double seqt);
};

wgen::wgen(int n, double eye_at_face) : eye_at_face(eye_at_face)
{
    eye_phi = randd() * pi * 2;
    for (int i=0; i<n; i++) {
        point p = o + sphere_random() * 2 * randd();
        double r = (randd() + 1) * .6;
        direction v = sphere_random() * 2 * randd();
        double h = randd() * pi * 2;
        bool is_bullet = 0 == i % 3;
        droplet_fixture f = { p, r, v, h, is_bullet };
        droplet_fixtures.push_back(f);
    }
}

world wgen::operator()(double seqt)
{
    vector<object> objects;
    for (auto f : droplet_fixtures) {
        const auto opts = ! f.is_bullet ? droplet_optics(f.h) : bullet_optics(f.h);
        const point ca = f.p + f.v * (seqt - .5);

        auto droplet_obj = object{ inter { sphere{ca, f.r} }, opts, {} };

        objects.push_back(droplet_obj);
    }

    double d = 8;
    double w = 3;
    // seems the correct observer for 3d glasses would be to leave
    // the 'screen' plane fixed, and have only the eye variable,
    // however for simpicity let's merely tilt on the view point
    double eye_phi = this->eye_phi + atan2(eye_at_face, d);
    const direction right = xyc(eye_phi + pi * .5);
    return {
        observer{ o + zd * 2 +- xyc(eye_phi) * d + right * w * eye_at_face, o, right * w },
       // rgb_sky,
          [](direction d) -> color { return white * .5 * (d.z + 1); },
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
    auto a = args(argc, argv, "E");
    a.run(wgen(16, strtof(a.get('E').c_str(), nullptr)));
}
