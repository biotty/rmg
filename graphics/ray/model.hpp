//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef MODEL_HPP
#define MODEL_HPP

#include "sky.h"
#include <vector>
#include <variant>
#include <optional>

namespace model {

struct color { double r, g, b; };
struct point { double x, y, z; };
struct direction { double x, y, z; };

constexpr point origo = {0, 0, 0};
constexpr point x1 = {1, 0, 0};
constexpr point y1 = {0, 1, 0};
constexpr point z1 = {0, 0, 1};
constexpr direction xd = {1, 0, 0};
constexpr direction yd = {0, 1, 0};
constexpr direction zd = {0, 0, 1};

point point_cast(direction d);
direction direction_cast(point p);
void mul_(point & p, double factor);
void mov_(point & p, direction offset);
void mul_(direction & d, const double m[9]);
void rot_(direction & d, direction axis, double angle);
void neg_(direction & d);
void rot_(point & p, point at, direction axis, double angle);

struct plane {
    point p;
    direction d;
    void _mul(double);
    void _mov(direction offset);
    void _rot(point at, direction axis, double angle);
};

struct sphere {
    point p;
    double r;
    void _mul(double factor);
    void _mov(direction offset);
    void _rot(point, direction, double);
};

struct common_geometric {
    point p;
    direction d;
    double r;
    void _mul(double factor);
    void _mov(direction offset);
    void _rot(point at, direction axis, double angle);
};
struct cylinder : common_geometric {};
struct cone : common_geometric {};
struct parabol : common_geometric {};
struct hyperbol : common_geometric {
    double h;
    void _mul(double factor);
};

struct saddle {
    point p;
    direction d;
    direction x;
    double h;
    void _mul(double factor);
    void _mov(direction offset);
    void _rot(point at, direction axis, double angle);
};

struct inv_shape {};
struct inv_plane : plane, inv_shape {};
struct inv_sphere : sphere, inv_shape {};
struct inv_cylinder : cylinder, inv_shape {};
struct inv_cone : cone, inv_shape {};
struct inv_hyperbol : hyperbol, inv_shape {};
struct inv_parabol : parabol, inv_shape {};
struct inv_saddle : saddle, inv_shape {};
using shape = std::variant<
    plane, inv_plane,
    sphere, inv_sphere,
    cylinder, inv_cylinder,
    cone, inv_cone,
    hyperbol, inv_hyperbol,
    parabol, inv_parabol,
    saddle, inv_saddle>;

void mul_(shape & s, double factor);
void mov_(shape & s, direction offset);
void rot_(shape & s, point at, direction axis, double angle);

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

using str = const char *;
struct angular {
    str n;
    surface s;
    direction d;
    double r;
    void _mul(double factor);
    void _mov(direction);
    void _rot(point, direction axis, double angle);
};

struct common_texture : common_geometric {
    str n;
    surface s;
};
struct planar : common_texture {};
struct planar1 : common_texture {};
struct relative : common_texture {};
struct axial : common_texture {};
struct axial1 : common_texture {};
struct checkers : common_geometric {
    int q;
    surface s;
};
using texture = std::variant<
    angular,
    planar, planar1,
    relative,
    axial, axial1,
    checkers>;

void mul_(texture & s, double factor);
void mov_(texture & s, direction offset);
void rot_(texture & s, point at, direction axis, double angle);

struct optics {
    color reflection_filter;
    color absorption_filter;
    double refraction_index;
    color refraction_filter;
    color passthrough_filter;
};

using inter = std::vector<shape>;
void mul_(inter & s, double factor);
void mov_(inter & s, direction offset);
void rot_(inter & s, point at, direction axis, double angle);

struct object {
    std::variant<shape, inter> s;
    optics o;
    std::optional<texture> u;
    object mul(double factor);
    object mov(direction offset);
    object rot(point at, direction axis, double angle);
};

struct light_spot {
    point p;
    color c;
};
using sky_function = void (*)(double * xyz_rgb);
struct world {
    observer obs;
    sky_function sky;
    std::vector<object> s;
    std::vector<light_spot> ls;
};
void render(
    const char * path, int width, int height,
    model::world w, unsigned n_threads = 0);

}
#endif
