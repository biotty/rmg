//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef RAYT_HPP
#define RAYT_HPP

#include "color.h"
#include "direction.h"
extern int trace_max_hops;
struct photo;

#include <array>
#include <vector>
#include <variant>
#include <optional>
#include <functional>

namespace rayt {

using m3 = std::array<double, 9>;
using color = ::color;
using point = ::point;
using direction = ::direction;
inline point point_cast(direction d) { return ::point_from_origo(d); }
inline direction direction_cast(point p) { return ::direction_from_origo(p); }
struct rotation { direction axis; double angle; };
struct resolution { unsigned width; unsigned height; };
constexpr resolution hdtv{1920, 1080};
constexpr double pi{3.14159265358979};
constexpr double tau{pi * 2};
constexpr double linear(double a, double b, double u) { return a * (1 - u) + b * u; }
constexpr double delinear(double a, double b, double u) { return (u - a) / (b - a); }
constexpr direction xd{1, 0, 0};
constexpr direction yd{0, 1, 0};
constexpr direction zd{0, 0, 1};
constexpr point o{0, 0, 0};
constexpr point xy(double x, double y) { return {x, y, 0}; }
constexpr point xz(double x, double z) { return {x, 0, z}; }
constexpr point yz(double y, double z) { return {0, y, z}; }
constexpr point onx(double k) { return {k, 0, 0}; }
constexpr point ony(double k) { return {0, k, 0}; }
constexpr point onz(double k) { return {0, 0, k}; }
constexpr color gray(double v) { return {v, v, v}; }
constexpr color red  {1, 0, 0};
constexpr color green{0, 1, 0};
constexpr color blue {0, 0, 1};
constexpr color white{1, 1, 1};
constexpr color black{0, 0, 0};
constexpr double water_ri{1.3};
constexpr double oil_ri{1.46};
constexpr double glass_ri{1.6};
constexpr double diamond_ri{2.4};
constexpr double silicon_ri{3.46};
constexpr double red_hue{0};
constexpr double yellow_hue{pi / 3};
constexpr double green_hue{tau / 3};
constexpr double blue_hue{2 * tau / 3};
constexpr double ultra_hue{tau};
color operator+(color p, color q);
color operator*(color p, color filter);
inline color operator*(color p, double u) { return p * gray(u); }
inline color from_hue(double h) { return from_hsv(h, 1, 1); }

direction operator*(direction, double);
direction operator-(direction);
direction operator+(direction a, direction b);
double operator*(direction a, direction b);
direction operator-(point to, point from);
point operator+(point, direction);
point & operator+=(point &, direction);
double abs(direction);
inline direction unit(direction d) { return d * (1 / abs(d)); }
direction xyc(double w);
direction xzc(double w);
direction yzc(double w);
inline double lin(double a, double b, double k) { return a * (1 - k) + b * k; }
inline color lin(color a, color b, double k) { return a * (1 - k) + b * k; }
inline point lin(point a, point b, double k)
{ return o + direction_cast(a) * (1 - k) + direction_cast(b) * k; }

void mov_(point & p, direction);
void mul_(direction & d, double);
void mul_(direction & d, m3);
void rot_(direction & d, rotation);
void rot_(point & p, point at, rotation);

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
    double r;
    direction d;
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
struct cuboid {
    point p;
    direction d;
    direction x;
    direction s;
    void _mul(double);
    void _mov(direction offset);
    void _rot(point at, rotation);
};

struct inv_shape {};
struct inv_plane : plane, inv_shape { inv_plane(point _p, direction _d); };
struct inv_sphere : sphere, inv_shape { inv_sphere(point _p, double _r); };
struct inv_cylinder : cylinder, inv_shape
{ inv_cylinder(point _p, double _r, direction _d); };
struct inv_cone : cone, inv_shape
{ inv_cone(point _p, double _r, direction _d); };
struct inv_parabol : parabol, inv_shape
{ inv_parabol(point _p, double _r, direction _d); };
struct inv_hyperbol : hyperbol, inv_shape
{ inv_hyperbol(point _p, double _r, direction _d, double _h); };
struct inv_saddle : saddle, inv_shape
{ inv_saddle(point _p, direction _d, direction _x, double _h); };
struct inv_cuboid : cuboid, inv_shape
{ inv_cuboid(point _p, direction _d, direction _x, direction _s); };

using shape = std::variant<
    plane, inv_plane,
    sphere, inv_sphere,
    cylinder, inv_cylinder,
    cone, inv_cone,
    hyperbol, inv_hyperbol,
    parabol, inv_parabol,
    saddle, inv_saddle,
    cuboid, inv_cuboid>;

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
    direction d;
    direction x;
    double r;
    str n;
    surface s;
    void _mul(double);
    void _mov(direction);
    void _rot(point, rotation);
}; 

struct common_texture : common_geometric {
    direction x;
    str n;
    surface s;
};
struct planar : common_texture {};
struct planar1 : common_texture {};
struct relative : common_texture {};
struct axial : common_texture {};
struct axial1 : common_texture {};
using texture = std::variant<
    angular,
    planar, planar1,
    relative,
    axial, axial1>;
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

using surface_f = std::function<surface(point, direction)>;

struct mapping_f
{
    point p;
    double r;
    direction d;
    direction x;
    surface_f f;
};
void mul_(mapping_f & m, point at, double);
void mov_(mapping_f & m, direction offset);
void rot_(mapping_f & m, point at, rotation);

using textmap = std::variant<texture, mapping_f>;

struct object {
    inter si;
    optics o;
    std::optional<textmap> u;
    object mul(point at, double);
    object mov(direction offset);
    object rot(point at, rotation);
};

struct light_spot {
    point p;
    color c;
};
using sky_f = std::function<color(direction)>;
struct world {
    observer obs;
    sky_f sky;
    std::vector<object> s;
    std::vector<light_spot> ls;
};
using world_gen_f = std::function<world(double)>;
void render(const world & w, std::string path, resolution, unsigned n_threads);
void sequence(world_gen_f wg, int nw, std::string path, resolution, unsigned n_threads);
struct args {
    const char * path = ""; resolution r = hdtv; double t = 0; int n = 0, j = 0;
    args(int argc, char ** argv, const std::string & extra = "");
    std::string get(char opt);
    void run(world_gen_f wg);
private:
    std::string values;
};

class photo_base {
    photo * ph;
protected:
    color _get(real x, real y);
public:
    virtual ~photo_base();
    photo_base(std::string path);
    photo_base(const photo_base & other);
};

struct photo_sky : photo_base {
    using photo_base::photo_base;
    color operator()(direction d);
};

namespace /* rayt:: */ solids {

constexpr double g = 1.61803398875;  // golden-ratio
constexpr double g2 = 2.618033988750235;  // golden-ratio ^ 2
constexpr double g3 = 4.236067977500615;  // golden-ratio ^ 3

static inline direction mean(std::initializer_list<direction> list)
{
    point mean = o;
    for (direction d : list) {
        mean += d;
    }
    return direction_cast(mean) * (1.0 / list.size());
}

extern direction tetra_faces[4];
constexpr double tetra_mr = 1.7320508075688772;
constexpr double tetra_cr = 3;

extern direction cube_faces[6];
constexpr double cube_mr = 1.4142135623730951;
constexpr double cube_cr = 1.7320508075688772;

extern direction octa_faces[8];
constexpr double octa_mr = 1.2247448713915892;
constexpr double octa_cr = 1.7320508075688772;

extern direction dodeca_faces[12];
constexpr double dodeca_mr = 1.1755705045849463;
constexpr double dodeca_cr = 1.258408572364819;

extern direction icosa_faces[20];
constexpr double icosa_mr = 1.07046626931927;
constexpr double icosa_cr = 1.258408572364819;

extern direction trunctetra_faces[8];
constexpr double trunctetra_cr = 1.9148542155126764;

extern direction truncocta_faces[14];
constexpr double truncocta_cr = 1.2909938468095037;

extern direction cubocta_faces[14];
constexpr double cubocta_cr = 1.4142135623730951;

extern direction trunccube_faces[14];
constexpr double trunccube_cr = 1.082392200292394;

extern direction truncdodeca_faces[32];
constexpr double truncdodeca_cr = 1.0929472010050334;

extern direction icosadodeca_faces[32];
constexpr double icosadodeca_cr = 1.1755743793385014;

extern direction truncicosa_faces[32];
constexpr double truncicosa_cr = 1.1940511957861941;

} // solids
} // rayt
#endif
