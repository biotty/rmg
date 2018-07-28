//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef MODEL_HPP
#define MODEL_HPP

#include "sky.h"
struct ray;
struct photo;
struct object_optics;

#include <array>
#include <vector>
#include <variant>
#include <optional>
#include <functional>

namespace model {

using m3 = std::array<double, 9>;
using color = ::color;
using point = ::point;
using direction = ::direction;
struct rotation { direction axis; double angle; };
struct resolution { int width; int height; };
constexpr resolution hdtv{1920, 1080};
constexpr double pi{3.14159265358979};
constexpr double pi2{pi * 2};
constexpr direction xd{1, 0, 0};
constexpr direction yd{0, 1, 0};
constexpr direction zd{0, 0, 1};
constexpr point o{0, 0, 0};
constexpr point point_cast(direction d) { return {d.x, d.y, d.z}; }
constexpr direction direction_cast(point p) { return {p.x, p.y, p.z}; }
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
constexpr double glass_ri{1.6};
constexpr double diamond_ri{2.4};
constexpr double red_hue{0};
constexpr double green_hue{pi2 / 3};
constexpr double blue_hue{2 * pi2 / 3};
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
double abs(direction);
inline direction norm(direction d) { return d * (1 / abs(d)); }
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
struct checkers : common_geometric {
    direction x;
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

struct mapping_f
{
    point p;
    double r;
    direction d;
    direction x;
    std::function<surface(point, direction)> f;
    void operator()(
            const ray &, object_optics &,
            const object_optics & adjust) const;
};
void mul_(mapping_f & m, point at, double);
void mov_(mapping_f & m, direction offset);
void rot_(mapping_f & m, point at, rotation);

using textmap = std::variant<texture, mapping_f>;

struct object {
    inter si;
    optics o;
    // todo: when textmap is simply mapping_f because the
    //       builtin textures has been moved out as
    //       mapping_f having respective photo_base impl
    //       as f member, the mapping_f operator() does not
    //       need adjust as unused, and needs the fixed
    //       passthru and refraction index.  then the above
    //       member optics o will instead be an alternative in
    //       the variant, so we get std::variant<optics, mapping_f>
    //       instead of o and the optional u.  the passthru and
    //       r-index could rather be members of mapping_f.
    //       with no builtin there is no use of the worlds
    //       decoration_args (world does not own a mapping_f
    //       as taken care of -- see above make function which
    //       does not push onto it for a mapping_f).
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
using world_gen_f = std::function<world(int i, int n)>;
void render(const world & w, std::string path, resolution, unsigned n_threads);
void sequence(world_gen_f wg, int nw, std::string path, resolution, unsigned n_threads);

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

}
#endif
