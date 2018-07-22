//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef MODEL_HPP
#define MODEL_HPP

#include "sky.h"
#include <array>
#include <vector>
#include <variant>
#include <optional>

namespace model {

struct color { double r, g, b; };
struct point { double x, y, z; };
struct direction { double x, y, z; };
struct rotation { direction axis; double angle; };
using m3 = std::array<double, 9>;
struct resolution { int width; int height; };
constexpr resolution hdtv{1920, 1080};
constexpr double pi{3.1415926535897};
constexpr double pi2{pi * 2};
constexpr point o = {0, 0, 0};
constexpr point xy(double x, double y) { return {x, y, 0}; }
constexpr point xz(double x, double z) { return {x, 0, z}; }
constexpr point yz(double y, double z) { return {0, y, z}; }
constexpr point onx(double k) { return {k, 0, 0}; }
constexpr point ony(double k) { return {0, k, 0}; }
constexpr point onz(double k) { return {0, 0, k}; }
constexpr direction xd = {1, 0, 0};
constexpr direction yd = {0, 1, 0};
constexpr direction zd = {0, 0, 1};

point point_cast(direction);
direction direction_cast(point);
void mov_(point & p, direction);
void mul_(direction & d, double);
void mul_(direction & d, m3);
void rot_(direction & d, rotation);
void rot_(point & p, point at, rotation);

direction operator*(direction d, double);
direction operator-(direction d);
direction operator+(direction a, direction b);
double operator*(direction a, direction b);
double abs(direction);
direction norm(direction d);
direction operator-(point to, point from);
point operator+(point, direction);

struct plane {
    point p;
    direction d;
    void _mul(double);
    void _mov(direction offset);
    void _rot(point at, rotation);
};

struct sphere {
    point p;
    double r;
    void _mul(double);
    void _mov(direction offset);
    void _rot(point at, rotation);
};

struct common_geometric {
    point p;
    direction d;
    double r;
    void _mul(double);
    void _mov(direction offset);
    void _rot(point at, rotation);
};
struct cylinder : common_geometric {};
struct cone : common_geometric {};
struct parabol : common_geometric {};
struct hyperbol : common_geometric {
    double h;
    void _mul(double);
};
struct saddle {
    point p;
    direction d;
    direction x;
    double h;
    void _mul(double);
    void _mov(direction offset);
    void _rot(point at, rotation);
};

struct inv_shape {};
struct inv_plane : plane, inv_shape { inv_plane(point _p, direction _d); };
struct inv_sphere : sphere, inv_shape { inv_sphere(point _p, double _r); };
struct inv_cylinder : cylinder, inv_shape
{ inv_cylinder(point _p, direction _d, double _r); };
struct inv_cone : cone, inv_shape
{ inv_cone(point _p, direction _d, double _r); };
struct inv_parabol : parabol, inv_shape
{ inv_parabol(point _p, direction _d, double _r); };
struct inv_hyperbol : hyperbol, inv_shape
{ inv_hyperbol(point _p, direction _d, double _r, double _h); };
struct inv_saddle : saddle, inv_shape
{ inv_saddle(point _p, direction _d, direction _x, double _h); };

using shape = std::variant<
    plane, inv_plane,
    sphere, inv_sphere,
    cylinder, inv_cylinder,
    cone, inv_cone,
    hyperbol, inv_hyperbol,
    parabol, inv_parabol,
    saddle, inv_saddle>;

void mul_(shape & s, double);
void mov_(shape & s, direction offset);
void rot_(shape & s, point at, rotation);

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

using str = std::string;

struct angular {
    str n;
    surface s;
    direction d;
    double r;
    void _mul(double);
    void _mov(direction);
    void _rot(point, rotation);
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

void mul_(texture & s, double);
void mov_(texture & s, direction offset);
void rot_(texture & s, point at, rotation);

struct optics {
    color reflection_filter;
    color absorption_filter;
    double refraction_index;
    color refraction_filter;
    color passthrough_filter;
};

using inter = std::vector<shape>;
void mul_(inter & s, point at, double);
void mov_(inter & s, direction offset);
void rot_(inter & s, point at, rotation);

struct object {
    inter si;
    optics o;
    std::optional<texture> u;
    object mul(point at, double);
    object mov(direction offset);
    object rot(point at, rotation);
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
void render(std::string path, resolution, world w, unsigned n_threads = 0);

}
#endif
