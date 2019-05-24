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

    object trunctetra{
        {
            sphere{ o, solids::trunctetra_cr *.96}
        }, poly_o, {} };

    for (auto d : solids::trunctetra_faces) {
        plane pl{ o + d, d };
        trunctetra.si.push_back(pl);
    }

    const double eye_angle = pi * seqt;
    rotation object_orientation{ poly_rotation_axis, seqt * tau };
    trunctetra = trunctetra.mul(o, .3).rot(o, object_orientation);

    return {
        observer{ o + xyc(eye_angle), o, xyc(eye_angle + pi * .5) },
        [](direction d) -> color {
            return { (d.x + 1), (d.y + 1), (d.z + 1) };
        },
        { trunctetra, }, {}
    };
}

int main(int argc, char ** argv)
{
    args(argc, argv).run(wgen);
}
