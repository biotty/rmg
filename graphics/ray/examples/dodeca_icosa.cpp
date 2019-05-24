#include "rayt.hpp"

#include <algorithm>

namespace
{
    const direction poly_rotation_axis = { .7, .3, -.5 };
}

using namespace rayt;

world wgen(double seqt)
{
    const optics poly_o{
        gray(.2),
        black,
        glass_ri,
        from_hsv(pi, .1, .85),
        from_hsv(pi, .2, .98)
    };
    struct indicator_optics : optics {
        indicator_optics(double hue)
            : optics{
                from_hsv(hue, .8, .7),
                black,
                water_ri,
                from_hsv(hue, .6, .8),
                white } {}
    };

    object icosa{
        {
            sphere{o, solids::icosa_cr},
            inv_sphere{o, solids::icosa_cr - .03},
        },
        indicator_optics{red_hue}, {}
    };

    const double dodeca_scale = linear(solids::icosa_cr, 1. / solids::dodeca_cr, seqt);

    object dodeca{
        {
            sphere{o, dodeca_scale * solids::dodeca_cr},
            inv_sphere{o, dodeca_scale * solids::dodeca_cr - .05 * (seqt - .5)},
        },
        indicator_optics{green_hue}, {}
    };

    object dodeca_icosa{ { sphere{o, solids::icosa_cr} }, poly_o, {} };

    for (auto d : solids::icosa_faces) {
        plane icosa_pl{ o + d, d};
        icosa.si.push_back(icosa_pl);
        dodeca_icosa.si.push_back(icosa_pl);
    }

    for (auto d : solids::dodeca_faces) {
        plane dodeca_pl{o + d * dodeca_scale, d};
        dodeca.si.push_back(dodeca_pl);
        dodeca_icosa.si.push_back(dodeca_pl);
    }

    const double eye_angle = pi * seqt + 1;
    const direction right = xyc(eye_angle + pi * .5);
    struct {
        object operator()(int i) {
            return {
                { sphere{b + t * (i - 3) * .15 + zd * .2 * (i % 2), .018 * (i + 5)} },
                indicator_optics{linear(green_hue, blue_hue,  .3 * i)},
                {} };
        }
        point b;
        direction t;
    } ball = { o +- xyc(eye_angle) * .4, right };
    const double scale = 1. / solids::icosa_cr;
    rotation object_orientation{ poly_rotation_axis, seqt * tau };
    dodeca = dodeca.mul(o, scale * (1 - TINY_REAL)).rot(o, object_orientation);
    icosa = icosa.mul(o, scale * (1 - TINY_REAL)).rot(o, object_orientation);
    dodeca_icosa = dodeca_icosa.mul(o, scale).rot(o, object_orientation);

    return {
        observer{ o + xyc(eye_angle), o, right },
        [](direction d) -> color {
            const double k = .5
                * std::clamp(std::abs(d.x + .3) * 98, 0., 1.)
                * std::clamp((.2 - d.z) * 6, 0., 1.);
            return {
                (1 + d.z) * k,
                (1 + d.y) * k,
                (1 + d.x)};
        },
        {
            ball(2), ball(3), ball(4), ball(5),
            dodeca_icosa, icosa, dodeca, }, {}
    };
}

int main(int argc, char ** argv)
{
    args(argc, argv).run(wgen);
}
