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
        direction rot_d_;
        double rot_a_;
        double rot_av_;
        int type;
        rotation rot(double t) {
            return { rot_d_, rot_a_ + rot_av_ * t };
        }
    };
    vector<droplet_fixture> droplet_fixtures;

    optics led_optics(double hue) {
        return {
            black,
            black,
            -10,
            from_hsv(hue, 1, 1),
            black
        };
    };

    optics bullet_optics(double hue) {
        return {
            from_hsv(hue, .1, .9),
            black,
            1.2,
            black,
            black
        };
    };

    optics droplet_optics(double hue) {
        return {
            from_hsv(hue, .1, .03),
            black,
            water_ri,
            from_hsv(hue, .2, .97),
            from_hsv(hue, .3, .99)
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
        double r = (randd() + 1) * .8;
        direction v = sphere_random() * 2 * randd();
        double h = randd() * pi * 2;
        int type = i % 11 - 3;
        if (type <= 0) {
            type = 0;
            v = v * .1;
            r *= .1;
        } else if (type > 2) {
            type = 2;
        }
        direction rot_d = sphere_random();
        double rot_a = randd() * pi * 2;
        double rot_av = randd() + .3;
        droplet_fixture f = { p, r, v, h, rot_d, rot_a, rot_av, type };
        droplet_fixtures.push_back(f);
    }
}

world wgen::operator()(double seqt)
{
    vector<object> objects;
    for (auto f : droplet_fixtures) {
        optics opts = {};
        optional<textmap> deco = {};
        switch (f.type) {
        case 0:
            opts = led_optics(f.h);
            break;
        case 1:
            opts = bullet_optics(f.h);
            /* alt.w function textmap: */
            deco = textmap_mapping(
                [](point p, direction d) -> surface {
                    (void) p;
                    auto v = abs(d.z);
                    constexpr auto u = 0.02;
                    return {
                        white * (v < u ? sin(pi * .5 * delinear(0, u, v)) : 1),
                        black,
                        black };
                }
            );
            rot_(std::get<mapping_f>(*deco), f.p, f.rot(seqt));
            /**/

            /* alt.w photo textmap:
            opts.reflection_filter = white * .5;
            deco = angular {
                zd, xd, 1, "../globe.jpeg",
                { white * .5, black, black }
            };
            rot_(std::get<texture>(*deco), f.p, f.rot(seqt));
            */
            break;
        case 2:
            opts = droplet_optics(f.h);
            break;
        default:
            assert(0);
        }
        const point ca = f.p + f.v * (seqt - .5);

        auto droplet_obj = object{ inter { sphere{ca, f.r} }, opts, deco };

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
        // [](direction d) -> color { return (rgb_sky(d) + white) * .3 * (d.z + 1); },
        // ^ factor should be .25 to maximize sky at white, this is boosted
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
    a.run(wgen(48, strtof(a.get('E').c_str(), nullptr)));
}
