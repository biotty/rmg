#include "rayt.hpp"

namespace
{
    const direction polyhedron_rotation_axis = { .7, .3, -.5 };
}

using namespace model;

world wgen(double seqt)
{
    color glass_c = from_hsv(blue_hue, .3, .95);
    optics glass_o{
        gray(.2),
        black,
        glass_ri,
        glass_c * .9,
        glass_c
    };
    optics ball_o{
        gray(.5),
        black,
        water_ri,
        white,
        gray(.5)
    };

    object cube_octahedron{
        {
            sphere{o, solids::octahedron_cr}
        },
        glass_o,
        {}
    };

    for (auto d : solids::octahedron_faces) {
        cube_octahedron.si.push_back(plane{ o + d, d });
    }

    double cube_ir = linear(solids::octahedron_cr, 1. / solids::cube_cr, seqt);

    for (auto d : solids::cube_faces) {
        auto p = o + d * cube_ir;
        cube_octahedron.si.push_back(plane{ p, d });
    }

    mul_(cube_octahedron.si, o, 1. / solids::octahedron_cr);
    rotation object_orientation{ polyhedron_rotation_axis, seqt * tau };

    double eye_angle = pi * seqt + .1;
    return {
        observer{ o + xyc(eye_angle), o, xyc(eye_angle + pi * .5) },
        [](direction d) -> color { return {
           (d.x + 1) * .5,
           (d.y + 1) * .5,
           (d.z + 1) * .5}; },
        {
            object{ { sphere{o +- xyc(eye_angle) * .3, .2} }, ball_o, {} },
            cube_octahedron.rot(o, object_orientation)
        },
        {}
    };
}

int main(int argc, char ** argv)
{
    main(wgen, argc, argv);
}
