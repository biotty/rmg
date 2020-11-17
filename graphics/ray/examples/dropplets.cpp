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

struct polar {
    double theta;
    double phi;
};

polar sphere_uniform(double u, double v)
{
    double theta = 2 * pi * u;
    double phi = acos(2 * v - 1);
    return { theta, phi };
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
    struct ball_fixture {
        point p;
        double r;
        direction v;
        double h;
    };
    vector<ball_fixture> ball_fixtures;

    struct ball_optics : optics {
        ball_optics(double hue) : optics{
            from_hsv(hue, .22, .11),
            black,
            water_ri,
            from_hsv(hue, .45, .98),
            from_hsv(hue, .6, .8) }
        {}
    };

    wgen(int n);
    world operator()(double seqt);
};

wgen::wgen(int n)
{
    eye_phi = randd() * pi * 2;
    for (int i=0; i<n; i++) {
        point p = o + sphere_random() * 6 * randd();
        double r = (randd() + 1) * .5;
        direction v = sphere_random() * 5 * randd();
        double h = randd() * pi * 2;
        ball_fixtures.push_back({ p, r, v, h });
    }
}

world wgen::operator()(double seqt)
{
    vector<object> balls;
    for (auto f : ball_fixtures) {
        object ball {
            { sphere{f.p + f.v * (seqt - .5), f.r} },
            ball_optics(f.h), {}
        };
        balls.push_back(ball);
    }

    const direction right = xyc(eye_phi + pi * .5);
    return {
        observer{ o +- xyc(eye_phi) * 8, o, right*3 }, rgb_sky,
        balls,
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
