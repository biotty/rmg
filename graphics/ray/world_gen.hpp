//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef MODEL_HPP
#define MODEL_HPP

#include "sky.h"
struct photo;

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
inline point point_cast(direction d) { return ::point_from_origo(d); }
inline direction direction_cast(point p) { return ::direction_from_origo(p); }
struct rotation { direction axis; double angle; };
struct resolution { int width; int height; };
constexpr resolution hdtv{1920, 1080};
constexpr double pi{3.14159265358979};
constexpr double pi2{pi * 2};
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
point & operator+=(point &, direction);
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

#ifdef MODEL_SOLIDS
namespace solids {

constexpr double g = 1.61803398875;  // golden-ratio
constexpr double g2 = 2.618033988750235;  // golden-ratio ^ 2
constexpr double g3 = 4.236067977500615;  // golden-ratio ^ 3

//  selection of vertices in 120-polyhedron
constexpr direction p2 { g2, 0 , g3};
constexpr direction p4 { 0 , g , g3};
constexpr direction p6 {-g2, 0 , g3};
constexpr direction p7 {-g ,-g2, g3};
constexpr direction p8 { 0 ,-g , g3};
constexpr direction p10{ g3, g , g2};
constexpr direction p11{ g2, g2, g2};
constexpr direction p12{ 0 , g3, g2};
constexpr direction p13{-g2, g2, g2};
constexpr direction p16{-g2,-g2, g2};
constexpr direction p17{ 0 ,-g3, g2};
constexpr direction p18{ g2,-g2, g2};
constexpr direction p20{ g3, 0 , g };
constexpr direction p22{-g2, g3, g };
constexpr direction p23{-g3, 0 , g };
constexpr direction p27{ g3, g2, 0 };
constexpr direction p28{ g , g3, 0 };
constexpr direction p30{-g , g3, 0 };
constexpr direction p31{-g3, g2, 0 };
constexpr direction p33{-g3,-g2, 0 };
constexpr direction p34{-g ,-g3, 0 };
constexpr direction p36{ g ,-g3, 0 };
constexpr direction p37{ g3,-g2, 0 };
constexpr direction p38{ g3, 0 ,-g };
constexpr direction p41{-g3, 0 ,-g };
constexpr direction p43{ g2,-g3,-g };
constexpr direction p45{ g2, g2,-g2};
constexpr direction p46{ 0 , g3,-g2};
constexpr direction p47{-g2, g2,-g2};
constexpr direction p49{-g3,-g ,-g2};
constexpr direction p50{-g2,-g2,-g2};
constexpr direction p51{ 0 ,-g3,-g2};
constexpr direction p52{ g2,-g2,-g2};
constexpr direction p54{ g2, 0 ,-g3};
constexpr direction p55{ g , g2,-g3};
constexpr direction p56{ 0 , g ,-g3};
constexpr direction p58{-g2, 0 ,-g3};
constexpr direction p60{ 0 ,-g ,-g3};

template <typename... Args>
direction mean(Args... directions)
{
    point mean = o;
    point dummy[] = {(mean += directions, o)...};
    (void)dummy;
    return direction_cast(mean) * (1.0 / sizeof...(directions));
}

direction tetrahedron_faces[] = {
        mean(p4, p34, p47),
        mean(p4, p38, p34),
        mean(p4, p47, p38),
        mean(p34, p38, p47)};

double tetrahedron_inradius = abs(tetrahedron_faces[0]);
double tetrahedron_midradius = 1.7321 * tetrahedron_inradius;
double tetrahedron_circumradius = 3 * tetrahedron_inradius;

direction cube_faces[] = {
    mean(p4, p18, p28, p38),
    mean(p4, p18, p23, p34),
    mean(p4, p23, p28, p47),
    mean(p28, p38, p47, p60),
    mean(p23, p34, p47, p60),
    mean(p18, p34, p38, p60)};

double cube_inradius = abs(cube_faces[0]);
double cube_midradius = 1.41422 * cube_inradius;
double cube_circumradius = 1.7321 * cube_inradius;

direction octahedron_faces[] = {
    mean(p7, p10, p43),
    mean(p7, p22, p10),
    mean(p7, p43, p49),
    mean(p7, p49, p22),
    mean(p55, p10, p43),
    mean(p55, p22, p10),
    mean(p55, p43, p49),
    mean(p55, p49, p22)};

double octahedron_inradius = abs(octahedron_faces[0]);
double octahedron_midradius = 1.2248 * octahedron_inradius;
double octahedron_circumradius = 1.7321 * octahedron_inradius;

direction dodecahedron_faces[] = {
    mean(p4, p8, p11, p18, p20),
    mean(p4, p8, p13, p16, p23),
    mean(p4, p11, p13, p28, p30),
    mean(p8, p16, p18, p34, p36),
    mean(p11, p20, p28, p38, p45),
    mean(p13, p23, p30, p41, p47),
    mean(p16, p23, p34, p41, p50),
    mean(p18, p20, p36, p38, p52),
    mean(p28, p30, p45, p47, p56),
    mean(p34, p36, p50, p52, p60),
    mean(p38, p45, p52, p56, p60),
    mean(p41, p47, p50, p56, p60)};

double dodecahedron_inradius = abs(dodecahedron_faces[0]);
double dodecahedron_midradius = 1.17557 * dodecahedron_inradius;
double dodecahedron_circumradius = 1.25841 * dodecahedron_inradius;

direction icosahedron_faces[] = {
    mean(p2, p6, p17),
    mean(p2, p12, p6),
    mean(p2, p17, p37),
    mean(p2, p37, p27),
    mean(p2, p27, p12),
    mean(p37, p54, p27),
    mean(p27, p54, p46),
    mean(p27, p46, p12),
    mean(p12, p46, p31),
    mean(p12, p31, p6),
    mean(p6, p31, p33),
    mean(p6, p33, p17),
    mean(p17, p33, p51),
    mean(p17, p51, p37),
    mean(p37, p51, p54),
    mean(p58, p54, p51),
    mean(p58, p46, p54),
    mean(p58, p31, p46),
    mean(p58, p33, p31),
    mean(p58, p51, p33)};

double icosahedron_inradius = abs(icosahedron_faces[0]);
double icosahedron_midradius = 1.07047 * icosahedron_inradius;
double icosahedron_circumradius = 1.25841 * icosahedron_inradius;
}
#endif

}
#endif
