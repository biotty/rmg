//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef MODEL_HPP
#define MODEL_HPP


#include <vector>
#include <variant>
#include <optional>


namespace model {

    struct color { double r, g, b; };
    struct point { double x, y, z; };
    struct direction { double x, y, z; };


    struct plane {
        point p;
        direction n;
    };


    struct sphere {
        point c;
        double r;
    }


    struct cylinder {
        point p;
        direction d;
        double r;
    };


    struct cone {
        point c;
        direction d;
        double r;
    };


    struct parabol {
        point f;
        point v;
    };


    struct hyperbol {
        point c;
        direction d;
        double r;
    };


    struct saddle {
        point c;
        direction z;
        direction d;
        double x, y;
    };


    using shape = std::variant<
        plane,
        sphere,
        cylinder,
        cone,
        parabol,
        hyperbol,
        saddle>;


    struct observer {
        point e;
        point c;
        direction x;
    };


    struct surface {
        color reflection;
        color absorption;
        color refraction;
    };


    struct angular {
        char * n;
        surface s;
        direction d;
        double r;
    };


#define TX(NAME)     \
    struct NAME {    \
        char * n;    \
        surface s;   \
        direction d; \
        double r;    \
        point p;     \
    };
    TX(planar)
    TX(planar1)
    TX(relative)
    TX(axial)
    TX(axial1)
#undef TX


    struct checkers {
        int q;
        surface s;
        direction d;
        double r;
        point p;
    };


    struct optics {
        reflection_filter;
        absorption_filter;
        refraction_index;
        refraction_filter;
        passthrough_filter;
    };


    struct object {
        optics o;
        std::vector<shape> m;
        std::optional<texture> u;
    };


    struct light_spot {
        point p;
        color c;
    };
}
#endif
