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
        gray(.15),
        black,
        glass_ri,
        from_hsv(blue_hue, .2, .85),
        from_hsv(blue_hue, .3, .98)
    };
    const optics ball_o{
        {.2, .1, .1},
        black,
        oil_ri,
        white,
        {.7, .2, .1}
    };
    struct indicator_optics : optics {
        indicator_optics(double hue)
            : optics{
                from_hsv(hue, .8, .7),
                black,
                diamond_ri,
                from_hsv(hue, .6, .8),
                white } {}
    };

    object octa{
        {
            sphere{o, solids::octa_cr},
            inv_sphere{o, solids::octa_cr - .09},
        },
        indicator_optics{red_hue}, {}
    };

    const double cube_scale = linear(solids::octa_cr, 1. / solids::cube_cr, seqt);

    object cube{
        {
            sphere{o, cube_scale * solids::cube_cr},
            inv_sphere{o, cube_scale * solids::cube_cr - .08},
        },
        indicator_optics{linear(red_hue, green_hue, .6)}, {}
    };

    object cube_octa{ { sphere{o, solids::octa_cr} }, poly_o, {} };

    for (auto d : solids::octa_faces) {
        plane octa_pl{ o + d, d};
        octa.si.push_back(octa_pl);
        cube_octa.si.push_back(octa_pl);
    }

    for (auto d : solids::cube_faces) {
        plane cube_pl{o + d * cube_scale, d};
        cube.si.push_back(cube_pl);
        cube_octa.si.push_back(cube_pl);
    }

    const double eye_angle = pi * seqt + .2;
    object ball{ { sphere{o +- xyc(eye_angle + .5) * .25, .25} }, ball_o, {} };
    const double scale = 1. / solids::octa_cr;
    rotation object_orientation{ poly_rotation_axis, seqt * tau };
    cube = cube.mul(o, scale * (1 - TINY_REAL)).rot(o, object_orientation);
    octa = octa.mul(o, scale * (1 - TINY_REAL)).rot(o, object_orientation);
    cube_octa = cube_octa.mul(o, scale).rot(o, object_orientation);

    return {
        observer{ o + xyc(eye_angle), o, xyc(eye_angle + pi * .5) },
        [](direction d) -> color {
            const double k = .5
                * std::clamp(std::abs(d.y - .05) * 94., 0., 1.)
                * std::clamp(std::abs(d.z - .45) * 1.9, 0., 1.);
            return {
                (d.x + 1) * k,
                (d.y + 1) * k,
                (d.z + 1) * k};
        },
        { ball, cube_octa, octa, cube, }, {}
    };
}

int main(int argc, char ** argv)
{
    args(argc, argv).run(wgen);
}
