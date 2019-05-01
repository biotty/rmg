#include "solids.hpp"

namespace
{
    const direction object_axis = { .3, -.5, .7 };
}

using namespace model;

world wgen(double seqt)
{
    color glass = from_hsv(blue_hue, .2, .9);

    object cube_octahedron{
        {
            sphere{o, solids::octahedron_cr}
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

    for (auto d : solids::octahedron_faces) {
        cube_octahedron.si.push_back(plane{ o + d, d });
    }

    double cube_ir = linear(1. / solids::cube_cr, solids::octahedron_cr, seqt);

    for (auto d : solids::cube_faces) {
        auto p = o + d * cube_ir;
        cube_octahedron.si.push_back(plane{ p, d });
    }

    mul_(cube_octahedron.si, o, 1. / solids::octahedron_cr);
    rotation object_orientation{ object_axis, seqt * tau };

    double eye_angle = pi * seqt + .1;
    world w{
        observer{ o + xyc(eye_angle), o, xyc(eye_angle + pi * .5) },
        [](direction d) -> color { return {
           (d.x + 1) * .5,
           (d.y + 1) * .5,
           (d.z + 1) * .5}; },
        {
            cube_octahedron.rot(o, object_orientation)
        },
        {}
    };

    return w;
}

int main(int argc, char ** argv)
{
    main(wgen, argc, argv);
}
