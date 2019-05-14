//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "rayt.hpp"
#include "trace.hpp"
#include "render.hpp"
#include "plane.h"
#include "sphere.h"
#include "cylinder.h"
#include "cone.h"
#include "parabol.h"
#include "hyperbol.h"
#include "saddle.h"
#include "inter.h"
#include "mapping.h"
#include "photo.h"
#include "observer.h"
#include "sky.h"

#include <cmath>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <type_traits>

namespace model {

color operator+(color p, color q)
{
    return {
        p.r + q.r,
        p.g + q.g,
        p.b + q.b};
}

color operator*(color p, color filter)
{
    return {
        p.r * filter.r,
        p.g * filter.g,
        p.b * filter.b};
}

color from_hsv(double h, double s, double v)
{
    if (s == 0) return gray(v);

    h = fmod(h, tau) * 3 / pi;
    int i = floor(h);
    double f = h - i;
    double p = v * (1 - s);
    double q = v * (1 - s * f);
    double t = v * (1 - s * (1 - f));
    switch (i) {
    case 0: return {v, t, p};
    case 1: return {q, v, p};
    case 2: return {p, v, t};
    case 3: return {p, q, v};
    case 4: return {t, p, v};
    case 5: return {v, p, q};
    default: abort();
    }
}

direction operator*(direction d, double factor)
{
    mul_(d, factor);
    return d;
}

direction operator-(direction d)
{
    return d * -1.0;
}

direction operator+(direction a, direction b)
{
    point ap = point_cast(a);
    mov_(ap, b);
    return direction_cast(ap);
}

double operator*(direction a, direction b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

direction operator-(point to, point from)
{
    mov_(to, -direction_cast(from));
    return direction_cast(to);
}

point operator+(point p, direction d)
{
    return point_cast(direction_cast(p) + d);
}

point & operator+=(point & p, direction d)
{
    p = p + d;
    return p;
}

double abs(direction d)
{
    return std::sqrt(d * d);
}

direction xyc(double w) { return {cos(w), sin(w), 0}; }
direction xzc(double w) { return {cos(w), 0, sin(w)}; }
direction yzc(double w) { return {0, cos(w), sin(w)}; }

    void
mov_(point & p, direction d)
{
    p.x += d.x;
    p.y += d.y;
    p.z += d.z;
}

    void
mul_(direction & d, double factor)
{
    d.x *= factor;
    d.y *= factor;
    d.z *= factor;
}

    void
mul_(direction & d, m3 m)
{
    double a[3] = { d.x, d.y, d.z };
    double r[3];
    for (int row=0; row<3; row++) {
        const double * m_row = & m[row * 3];
        double s = 0;
        for (int column=0; column<3; column++)
            s += m_row[column] * a[column];
        r[row] = s;
    }
    d.x = r[0];
    d.y = r[1];
    d.z = r[2];
}

    void
rot_(direction & d, rotation ro)
{
    const direction u = unit(ro.axis);
    const double c = std::cos(ro.angle);
    const double s = std::sin(ro.angle);
    const double i = 1 - c;
    const m3 m{
        u.x*u.x + (1 - u.x*u.x)*c, u.x*u.y*i + u.z*s, u.x*u.z*i - u.y*s,
        u.x*u.y*i - u.z*s, u.y*u.y+(1 - u.y*u.y)*c, u.y*u.z*i + u.x*s,
        u.x*u.z*i + u.y*s, u.y*u.z*i - u.x*s,  u.z*u.z + (1 - u.z*u.z)*c
    };
    mul_(d, m);
}

    void
rot_(point & p, point at, rotation ro)
{
    direction d = direction_cast(at);
    mov_(p, -d);
    direction q = direction_cast(p);
    rot_(q, ro);
    p = point_cast(q);
    mov_(p, d);
}

void plane::_mul(double) {}
void plane::_mov(direction offset) { mov_(p, offset); }
void plane::_rot(point at, rotation ro)
{
    rot_(p, at, ro);
    rot_(d, ro);
}

void sphere::_mul(double factor) { r *= factor; }
void sphere::_mov(direction offset) { mov_(p, offset); }
void sphere::_rot(point at, rotation ro)
{
    rot_(p, at, ro);
}

void common_geometric::_mul(double factor) { r *= factor; }
void common_geometric::_mov(direction offset) { mov_(p, offset); }
void common_geometric::_rot(point at, rotation ro)
{
    rot_(p, at, ro);
    rot_(d, ro);
}

void hyperbol::_mul(double factor) {
    r *= factor;
    h *= factor;
}

void saddle::_mul(double factor) { h *= factor; }
void saddle::_mov(direction offset) { mov_(p, offset); }
void saddle::_rot(point at, rotation ro)
{
    rot_(p, at, ro);
    rot_(d, ro);
    rot_(x, ro);
}

inv_plane::inv_plane(point _p, direction _d) : plane{_p, _d} {}
inv_sphere::inv_sphere(point _p, double _r) : sphere{_p, _r} {}
inv_cylinder::inv_cylinder(point _p, double _r, direction _d) : cylinder{_p, _r, _d} {}
inv_cone::inv_cone(point _p, double _r, direction _d) : cone{_p, _r, _d} {}
inv_parabol::inv_parabol(point _p, double _r, direction _d) : parabol{_p, _r, _d} {}
inv_hyperbol::inv_hyperbol(point _p, double _r, direction _d, double _h) : hyperbol{_p, _r, _d, _h} {}
inv_saddle::inv_saddle(point _p, direction _d, direction _x, double _h) : saddle{_p, _d, _x, _h} {}

void mul_(shape & s, double factor)
{
    std::visit([factor](auto & arg) { arg._mul(factor); }, s);
}

void mov_(shape & s, direction offset)
{
    std::visit([offset](auto & arg) { arg._mov(offset); }, s);
}

void rot_(shape & s, point at, rotation ro)
{
    std::visit([at, ro](auto & arg) { arg._rot(at, ro); }, s);
}

void angular::_mul(double factor) { r *= factor; }
void angular::_mov(direction) {}
void angular::_rot(point, rotation ro) { rot_(d, ro); }

void mov_(angular &, point, double) {}
void mov_(common_geometric & arg, point at, double factor)
{ arg.p = at + (arg.p - at) * factor; }

    void
mul_(texture & s, point at, double factor)
{
    std::visit([at, factor](auto & arg) {
            arg._mul(factor);
            mov_(arg, at, factor);
            }, s);
}

    void
mov_(texture & s, direction offset)
{
    std::visit([offset](auto & arg) { arg._mov(offset); }, s);
}

    void
rot_(texture & s, point at, rotation ro)
{
    std::visit([at, ro](auto & arg) { arg._rot(at, ro); }, s);
}

void mul_(mapping_f & m, point at, double factor)
{
    m.r *= factor;
    m.p = at + (m.p - at) * factor;
}

void mov_(mapping_f & m, direction offset)
{
    mov_(m.p, offset);
}

void rot_(mapping_f & m, point at, rotation ro)
{
    rot_(m.p, at, ro);
    rot_(m.d, ro);
    rot_(m.x, ro);
}

    void
mul_(inter & s, point at, double factor) {
    for (auto & o : s)
        std::visit([at, factor](auto & arg) {
                arg._mul(factor);
                arg.p = at + (arg.p - at) * factor; }, o);
}

    void
mov_(inter & s, direction offset) {
    for (auto & o : s)
        std::visit([offset](auto & arg) { arg._mov(offset); }, o);
}

    void
rot_(inter & s, point at, rotation ro) {
    for (auto & o : s)
        std::visit([at, ro](auto & arg) { arg._rot(at, ro); }, o);
}

    object
object::mul(point at, double factor)
{
    object ret = *this;
    mul_(ret.si, at, factor);
    if (ret.u) std::visit([at, factor](auto & arg) { mul_(arg, at, factor); }, *ret.u);
    return ret;
}

    object
object::mov(direction offset)
{
    object ret = *this;
    mov_(ret.si, offset);
    if (ret.u) std::visit([offset](auto & arg) { mov_(arg, offset); }, *ret.u);
    return ret;
}

    object
object::rot(point at, rotation ro)
{
    object ret = *this;
    rot_(ret.si, at, ro);
    if (ret.u) std::visit([at, ro](auto & arg) { rot_(arg, at, ro); }, *ret.u);
    return ret;
}

}

namespace {

    direction
make_unit(model::direction d)
{
    normalize(&d);
    return d;
}

    tilt_arg
make_tilt(model::direction d)
{
    tilt_arg rota;
    real ignore_r;
    spherical_arg(d, &ignore_r, &rota);
    return rota;
}

    base_arg
make_base(model::direction d, model::direction x)
{
    return { make_unit(x), make_unit(d) };
}

    const char * // lifetime: str (in model::world object)
make(const model::str & s) {
    return s.c_str();
}

    void *
make(model::plane pl, object_intersection * fi, object_normal * fn, bool inv)
{
        auto plane_ = alloc<plane>();
        plane_->at = pl.p;
        plane_->normal = make_unit(pl.d);
        *fi = plane_intersection;
        *fn = plane_normal;
        if (inv) {
            scale(&plane_->normal, -1);
        }
        return plane_;
}

    void *
make(model::sphere sp, object_intersection * fi, object_normal * fn, bool inv)
{
    auto sphere_ = alloc<sphere>();
    sphere_->center = sp.p;
    sphere_->sq_radius = square(sp.r);
    if (inv) {
        *fi = _sphere_intersection;
        *fn = _sphere_normal;
    } else {
        *fi = sphere_intersection;
        *fn = sphere_normal;
    }
    return sphere_;
}

    void *
make(model::cylinder cy, object_intersection * fi, object_normal * fn, bool inv)
{
    auto cylinder_ = alloc<cylinder>();
    *cylinder_ = cylinder{
        distance_vector(cy.p, origo),
            (float)square(cy.r), make_tilt(cy.d)};
    if (inv) {
        *fi = _cylinder_intersection;
        *fn = _cylinder_normal;
    } else {
        *fi = cylinder_intersection;
        *fn = cylinder_normal;
    }
    return cylinder_;
}

    void *
make(model::cone co, object_intersection * fi, object_normal * fn, bool inv)
{
    auto cone_ = alloc<cone>();
    *cone_ = cone{
        distance_vector(co.p, origo),
            1/(float)co.r, make_tilt(co.d)};
    if (inv) {
        *fi = _cone_intersection;
        *fn = _cone_normal;
    } else {
        *fi = cone_intersection;
        *fn = cone_normal;
    }
    return cone_;
}

    void *
make(model::parabol pa, object_intersection * fi, object_normal * fn, bool inv)
{
    auto parabol_ = alloc<parabol>();
    direction f = make_unit(pa.d);
    scale(&f, pa.r * .5);
    *parabol_ = parabol{
        distance_vector(pa.p, point_from_origo(f)),
        (float) pa.r, make_tilt(pa.d)};
    if (inv) {
        *fi = _parabol_intersection;
        *fn = _parabol_normal;
    } else {
        *fi = parabol_intersection;
        *fn = parabol_normal;
    }
    return parabol_;
}

    void *
make(model::hyperbol hy, object_intersection * fi, object_normal * fn, bool inv)
{
    auto hyperbol_ = alloc<hyperbol>();
    *hyperbol_ = hyperbol{
        distance_vector(hy.p, origo),
            make_tilt(hy.d), 1/(float)hy.h, 1/(float)hy.r};
    if (inv) {
        *fi = _hyperbol_intersection;
        *fn = _hyperbol_normal;
    } else {
        *fi = hyperbol_intersection;
        *fn = hyperbol_normal;
    }
    return hyperbol_;
}

    void *
make(model::saddle sa, object_intersection * fi, object_normal * fn, bool inv)
{
    auto saddle_ = alloc<saddle>();
    *saddle_ = saddle{
        distance_vector(sa.p, origo),
            make_base(sa.d, sa.x),
            1 /(float) sa.h};
    *fi = saddle_intersection;
    *fn = saddle_normal;
    if (inv) {
        saddle_->h *= -1;
        saddle_->base.x = cross(saddle_->base.z, saddle_->base.x);
    }
    return saddle_;
}

    void *
make(model::shape sh, object_intersection * fi, object_normal * fn)
{
    // note: different overload(/name) prevents that if function for variant-
    //       alternative is omitted by mistake then we get compile error
    //       instead of silent setup for infinite recursion by overload-
    //       match via implicit conversion by variant constructor
    void * ret;
    std::visit([&ret, fi, fn](auto arg) {
            constexpr bool inv = std::is_base_of<model::inv_shape, decltype(arg)>::value;
            ret = make(arg, fi, fn, inv);
            }, sh);
    return ret;
}

    void *
member_get(object_intersection * fi, object_normal * fn, void * state)
{
    auto pp = static_cast<model::shape **>(state);
    return make(*(*pp)++, fi, fn);
}

    void *
make(model::inter in, object_intersection * fi, object_normal * fn, world & owner_)
{
    const size_t n = in.size();
    model::shape * ptr = &in[0];
    if (n == 0) return nullptr;

    void * ret;
    if (n == 1) {
        ret = make(*ptr, fi, fn);
        owner_.object_args.push_back(ret);
    } else {
        ret = make_inter(fi, fn, n, member_get, &ptr);
        owner_.inter_args.push_back(ret);
    }
    return ret;
}

    texture_application
make(model::surface s)
{
    return {
        s.reflection,
        s.absorption,
        s.refraction
    };
}

    void *
make(model::angular tx, object_decoration * df)
{
    return angular_texture_mapping(df, make_base(tx.d, tx.x),
            tx.r, make(tx.n), make(tx.s));
}

    void *
make(model::planar tx, object_decoration * df)
{
    return planar_texture_mapping(df, make_base(tx.d, tx.x),
            tx.r, tx.p, make(tx.n), make(tx.s));
}

    void *
make(model::planar1 tx, object_decoration * df)
{
    return planar1_texture_mapping(df, make_base(tx.d, tx.x),
            tx.r, tx.p, make(tx.n), make(tx.s));
}

    void *
make(model::relative tx, object_decoration * df)
{
    return relative_texture_mapping(df, make_base(tx.d, tx.x),
            tx.r, tx.p, make(tx.n), make(tx.s));
}

    void *
make(model::axial tx, object_decoration * df)
{
    return axial_texture_mapping(df, make_base(tx.d, tx.x),
            tx.r, tx.p, make(tx.n), make(tx.s));
}

    void *
make(model::axial1 tx, object_decoration * df)
{
    return axial1_texture_mapping(df, make_base(tx.d, tx.x),
            tx.r, tx.p, make(tx.n), make(tx.s));
}

    compact_color
make_c(model::color c)
{ return z_filter(c); }

struct surface_decoration_arg {
    point o;
    real r;
    base_arg base;
    model::surface_f const & f;
};

    static void
surface_decoration_(const ray * ray_, const void * arg,
        object_optics * so, const object_optics * adjust)
{
    auto da = static_cast<const surface_decoration_arg *>(arg);
    auto [p, d] = *ray_;
    p = point_from_origo(inverse_base(distance_vector(da->o, p), da->base));
    d = inverse_base(d, da->base);
    scale(&d, da->r);
    model::surface s = da->f(p, d);
    so->refraction_index = adjust->refraction_index;
    so->passthrough_filter = adjust->passthrough_filter;
    // improve: could encode in s usage of adjust (unused)
    so->reflection_filter = z_filter(s.reflection);
    so->absorption_filter = z_filter(s.absorption);
    so->refraction_filter = z_filter(s.refraction);
}

    void *
make(model::textmap const & tm, object_decoration * df, world & world_)
{
    void * da;
    if (std::holds_alternative<model::texture>(tm)) {
        auto tx = std::get<model::texture>(tm);
        std::visit([&da, df, &world_](auto arg){
                da = make(arg, df); }, tx);
    } else {
        auto const & mf = std::get<model::mapping_f>(tm);
        using D = surface_decoration_arg;
        da = new (alloc<D>()) D{mf.p, 1 / mf.r, make_base(mf.d, mf.x), mf.f};
        *df = surface_decoration_;
    }
    world_.decoration_args.push_back(da);
    return da;
}

    object_optics
make(model::optics o)
{
    return object_optics{
        static_cast<float>(o.refraction_index),
        make_c(o.reflection_filter),
        make_c(o.absorption_filter),
        make_c(o.refraction_filter),
        make_c(o.passthrough_filter)
    };
}

    void
make(model::object const & obj, world & world_)
{
    object_intersection fi;
    object_normal fn;
    object_decoration fd = nullptr;
    auto a = make(obj.si, &fi, &fn, world_);
    auto d = obj.u ? make(*obj.u, &fd, world_) : nullptr;
    auto o = scene_object{ fi, fn, a, make(obj.o), fd, d };
    world_.scene_.push_back(o);
}

    observer
make(model::observer o)
{
    observer ret;
    ret.eye = o.e;
    ret.view = o.c;
    ret.column_direction = o.x;
    // todo: adjust column so perpendicular to view-eye
    direct_row(&ret);
    return ret;
}

    static color
sky_(direction d)
{
    return (*static_cast<model::sky_f *>(sky_arg))(d);
}

    std::pair<observer, world>
make(const model::world & w)
{
    sky_arg = const_cast<model::sky_f *>(&w.sky);
    std::pair<observer, world> ret{
        make(w.obs), world{sky_, delete_inter, delete_decoration}
    };
    for (auto & o : w.s) make(o, ret.second);
    for (auto & s : w.ls)
        ret.second.spots_.push_back({s.p, s.c});
    return ret;
}

}

namespace model {

void render(const model::world & w, std::string path, resolution res, unsigned n_threads) {
    auto [obs, world_] = make(w);
    render(path.c_str(), res.width, res.height, obs, world_, n_threads);
    sky_arg = nullptr;  // init: by make(w)
}

void sequence(world_gen_f wg, int n_frames, std::string path, resolution res, unsigned n_threads)
{
    for (int i = 0; i < n_frames; i++) {
        std::ostringstream buf{path, std::ostringstream::ate};
        buf << i << ".jpeg";
        render(wg(i /(double) n_frames), buf.str(), res, n_threads);
        std::cout << "\r" << i << std::flush;
    }
    std::cout << std::endl;
}

static resolution parse_resolution(char *s)
{
    resolution r;
    if (2 != sscanf(s, "%ux%u", &r.width, &r.height)
            || r.width == 0 || r.height == 0
            || r.width > 99999 || r.height > 99999) {
        r = hdtv;
        std::cerr << "using resolution " << s << "\n";
    }
    return r;
}

void main(world_gen_f wg, int argc, char ** argv)
{
    const char * path = "";
    resolution r = hdtv;
    double t = 0;
    int n = 0;
    int j = 0;
    int opt;
    while ((opt = getopt(argc, argv, "j:n:r:t:")) != -1) {
        switch (opt) {
        case 'n':
            n = atoi(optarg);
            break;
        case 'r':
            r = parse_resolution(optarg);
            break;
        case 't':
            t = atof(optarg);
            break;
        case 'j':
            j = atoi(optarg);
            if (j > 64) {
                j = 64;
                std::cerr << "clamping to " << j << " threads\n";
            }
            break;
        default:
            std::cerr << "Usage: %s [-j jobs] [-n frames]  [-r WxH] [-t 0..1] [path]";
            exit(EXIT_FAILURE);
        }
    }
    if (optind < argc) path = argv[optind];

    if (n) sequence(wg, n, path, r, 0);
    else render(wg(t), path, r, 0);
}

color photo_base::_get(real x, real y)
{
    const auto a = reinterpret_cast<photo_attr *>(ph);
    const real col = (x == 0) ? a->width - 1 : (1 - x) * a->width;
    // ^ horizontally flip as we see the "sphere" from the "inside"
    compact_color cc = photo_color(ph, col, y * a->height);
    return x_color(cc);
}
photo_base::photo_base(std::string path) : ph(photo_create(path.c_str())) {}
photo_base::photo_base(const photo_base & other) : ph(other.ph) { photo_incref(ph); }
photo_base::~photo_base() { photo_delete(ph); }

color photo_sky::operator()(direction d)
{
    real x, y;
    direction_to_unitsquare(&d, &x, &y);
    return _get(x, y);
}

namespace /* model:: */ solids {

//  selection of vertices in 120-polyhedron
direction p2 { g2, 0 , g3};
direction p4 { 0 , g , g3};
direction p6 {-g2, 0 , g3};
direction p7 {-g ,-g2, g3};
direction p8 { 0 ,-g , g3};
direction p10{ g3, g , g2};
direction p11{ g2, g2, g2};
direction p12{ 0 , g3, g2};
direction p13{-g2, g2, g2};
direction p16{-g2,-g2, g2};
direction p17{ 0 ,-g3, g2};
direction p18{ g2,-g2, g2};
direction p20{ g3, 0 , g };
direction p22{-g2, g3, g };
direction p23{-g3, 0 , g };
direction p27{ g3, g2, 0 };
direction p28{ g , g3, 0 };
direction p30{-g , g3, 0 };
direction p31{-g3, g2, 0 };
direction p33{-g3,-g2, 0 };
direction p34{-g ,-g3, 0 };
direction p36{ g ,-g3, 0 };
direction p37{ g3,-g2, 0 };
direction p38{ g3, 0 ,-g };
direction p41{-g3, 0 ,-g };
direction p43{ g2,-g3,-g };
direction p45{ g2, g2,-g2};
direction p46{ 0 , g3,-g2};
direction p47{-g2, g2,-g2};
direction p49{-g3,-g ,-g2};
direction p50{-g2,-g2,-g2};
direction p51{ 0 ,-g3,-g2};
direction p52{ g2,-g2,-g2};
direction p54{ g2, 0 ,-g3};
direction p55{ g , g2,-g3};
direction p56{ 0 , g ,-g3};
direction p58{-g2, 0 ,-g3};
direction p60{ 0 ,-g ,-g3};

direction tetra_faces[4] = {
        unit(mean({p4, p34, p47})),
        unit(mean({p4, p38, p34})),
        unit(mean({p4, p47, p38})),
        unit(mean({p34, p38, p47}))};

direction cube_faces[6] = {
    unit(mean({p4, p18, p28, p38})),
    unit(mean({p4, p18, p23, p34})),
    unit(mean({p4, p23, p28, p47})),
    unit(mean({p28, p38, p47, p60})),
    unit(mean({p23, p34, p47, p60})),
    unit(mean({p18, p34, p38, p60}))};

direction octa_faces[8] = {
    unit(mean({p7, p10, p43})),
    unit(mean({p7, p22, p10})),
    unit(mean({p7, p43, p49})),
    unit(mean({p7, p49, p22})),
    unit(mean({p55, p10, p43})),
    unit(mean({p55, p22, p10})),
    unit(mean({p55, p43, p49})),
    unit(mean({p55, p49, p22}))};

direction dodeca_faces[12] = {
    unit(mean({p4, p8, p11, p18, p20})),
    unit(mean({p4, p8, p13, p16, p23})),
    unit(mean({p4, p11, p13, p28, p30})),
    unit(mean({p8, p16, p18, p34, p36})),
    unit(mean({p11, p20, p28, p38, p45})),
    unit(mean({p13, p23, p30, p41, p47})),
    unit(mean({p16, p23, p34, p41, p50})),
    unit(mean({p18, p20, p36, p38, p52})),
    unit(mean({p28, p30, p45, p47, p56})),
    unit(mean({p34, p36, p50, p52, p60})),
    unit(mean({p38, p45, p52, p56, p60})),
    unit(mean({p41, p47, p50, p56, p60}))};

direction icosa_faces[20] = {
    unit(mean({p2, p6, p17})),
    unit(mean({p2, p12, p6})),
    unit(mean({p2, p17, p37})),
    unit(mean({p2, p37, p27})),
    unit(mean({p2, p27, p12})),
    unit(mean({p37, p54, p27})),
    unit(mean({p27, p54, p46})),
    unit(mean({p27, p46, p12})),
    unit(mean({p12, p46, p31})),
    unit(mean({p12, p31, p6})),
    unit(mean({p6, p31, p33})),
    unit(mean({p6, p33, p17})),
    unit(mean({p17, p33, p51})),
    unit(mean({p17, p51, p37})),
    unit(mean({p37, p51, p54})),
    unit(mean({p58, p54, p51})),
    unit(mean({p58, p46, p54})),
    unit(mean({p58, p31, p46})),
    unit(mean({p58, p33, p31})),
    unit(mean({p58, p51, p33}))};

direction trunctetra_faces[8] = {
    octa_faces[0],
    octa_faces[1] * (5 / 3.),
    octa_faces[2] * (5 / 3.),
    octa_faces[3],
    octa_faces[4] * (5 / 3.),
    octa_faces[5],
    octa_faces[6],
    octa_faces[7] * (5 / 3.)};

constexpr double truncocta_ratio = linear(cube_cr, 1. / cube_cr, 1 / 2.);
direction truncocta_faces[14] = {
    cube_faces[0] * truncocta_ratio,
    cube_faces[1] * truncocta_ratio,
    cube_faces[2] * truncocta_ratio,
    cube_faces[3] * truncocta_ratio,
    cube_faces[4] * truncocta_ratio,
    cube_faces[5] * truncocta_ratio,
    octa_faces[0],
    octa_faces[1],
    octa_faces[2],
    octa_faces[3],
    octa_faces[4],
    octa_faces[5],
    octa_faces[6],
    octa_faces[7]};

constexpr double cubocta_ratio = 1. / linear(cube_cr, 1. / cube_cr, 3 / 4.);
direction cubocta_faces[14] = {
    cube_faces[0],
    cube_faces[1],
    cube_faces[2],
    cube_faces[3],
    cube_faces[4],
    cube_faces[5],
    octa_faces[0] * cubocta_ratio,
    octa_faces[1] * cubocta_ratio,
    octa_faces[2] * cubocta_ratio,
    octa_faces[3] * cubocta_ratio,
    octa_faces[4] * cubocta_ratio,
    octa_faces[5] * cubocta_ratio,
    octa_faces[6] * cubocta_ratio,
    octa_faces[7] * cubocta_ratio};

constexpr double trunccube_ratio = 1. / linear(cube_cr, 1. / cube_cr, 7 / 8.);
direction trunccube_faces[14] = {
    cube_faces[0],
    cube_faces[1],
    cube_faces[2],
    cube_faces[3],
    cube_faces[4],
    cube_faces[5],
    octa_faces[0] * trunccube_ratio,
    octa_faces[1] * trunccube_ratio,
    octa_faces[2] * trunccube_ratio,
    octa_faces[3] * trunccube_ratio,
    octa_faces[4] * trunccube_ratio,
    octa_faces[5] * trunccube_ratio,
    octa_faces[6] * trunccube_ratio,
    octa_faces[7] * trunccube_ratio};

constexpr double truncdodeca_ratio = linear(icosa_cr, 1. / icosa_cr, 1 / 2.);
direction truncdodeca_faces[32] = {
    dodeca_faces[0] * truncdodeca_ratio,
    dodeca_faces[1] * truncdodeca_ratio,
    dodeca_faces[2] * truncdodeca_ratio,
    dodeca_faces[3] * truncdodeca_ratio,
    dodeca_faces[4] * truncdodeca_ratio,
    dodeca_faces[5] * truncdodeca_ratio,
    dodeca_faces[6] * truncdodeca_ratio,
    dodeca_faces[7] * truncdodeca_ratio,
    dodeca_faces[8] * truncdodeca_ratio,
    dodeca_faces[9] * truncdodeca_ratio,
    dodeca_faces[10] * truncdodeca_ratio,
    dodeca_faces[11] * truncdodeca_ratio,
    icosa_faces[0],
    icosa_faces[1],
    icosa_faces[2],
    icosa_faces[3],
    icosa_faces[4],
    icosa_faces[5],
    icosa_faces[6],
    icosa_faces[7],
    icosa_faces[8],
    icosa_faces[9],
    icosa_faces[10],
    icosa_faces[11],
    icosa_faces[12],
    icosa_faces[13],
    icosa_faces[14],
    icosa_faces[15],
    icosa_faces[16],
    icosa_faces[17],
    icosa_faces[18],
    icosa_faces[19]};

constexpr double icosadodeca_ratio = 1. / linear(icosa_cr, 1. / icosa_cr, 3 / 4.);
direction icosadodeca_faces[32] = {
    dodeca_faces[0],
    dodeca_faces[1],
    dodeca_faces[2],
    dodeca_faces[3],
    dodeca_faces[4],
    dodeca_faces[5],
    dodeca_faces[6],
    dodeca_faces[7],
    dodeca_faces[8],
    dodeca_faces[9],
    dodeca_faces[10],
    dodeca_faces[11],
    icosa_faces[0] * icosadodeca_ratio,
    icosa_faces[1] * icosadodeca_ratio,
    icosa_faces[2] * icosadodeca_ratio,
    icosa_faces[3] * icosadodeca_ratio,
    icosa_faces[4] * icosadodeca_ratio,
    icosa_faces[5] * icosadodeca_ratio,
    icosa_faces[6] * icosadodeca_ratio,
    icosa_faces[7] * icosadodeca_ratio,
    icosa_faces[8] * icosadodeca_ratio,
    icosa_faces[9] * icosadodeca_ratio,
    icosa_faces[10] * icosadodeca_ratio,
    icosa_faces[11] * icosadodeca_ratio,
    icosa_faces[12] * icosadodeca_ratio,
    icosa_faces[13] * icosadodeca_ratio,
    icosa_faces[14] * icosadodeca_ratio,
    icosa_faces[15] * icosadodeca_ratio,
    icosa_faces[16] * icosadodeca_ratio,
    icosa_faces[17] * icosadodeca_ratio,
    icosa_faces[18] * icosadodeca_ratio,
    icosa_faces[19] * icosadodeca_ratio};

constexpr double truncicosa_ratio = 1. / linear(icosa_cr, 1. / icosa_cr, 7 / 8.);
direction truncicosa_faces[32] = {
    dodeca_faces[0],
    dodeca_faces[1],
    dodeca_faces[2],
    dodeca_faces[3],
    dodeca_faces[4],
    dodeca_faces[5],
    dodeca_faces[6],
    dodeca_faces[7],
    dodeca_faces[8],
    dodeca_faces[9],
    dodeca_faces[10],
    dodeca_faces[11],
    icosa_faces[0] * truncicosa_ratio,
    icosa_faces[1] * truncicosa_ratio,
    icosa_faces[2] * truncicosa_ratio,
    icosa_faces[3] * truncicosa_ratio,
    icosa_faces[4] * truncicosa_ratio,
    icosa_faces[5] * truncicosa_ratio,
    icosa_faces[6] * truncicosa_ratio,
    icosa_faces[7] * truncicosa_ratio,
    icosa_faces[8] * truncicosa_ratio,
    icosa_faces[9] * truncicosa_ratio,
    icosa_faces[10] * truncicosa_ratio,
    icosa_faces[11] * truncicosa_ratio,
    icosa_faces[12] * truncicosa_ratio,
    icosa_faces[13] * truncicosa_ratio,
    icosa_faces[14] * truncicosa_ratio,
    icosa_faces[15] * truncicosa_ratio,
    icosa_faces[16] * truncicosa_ratio,
    icosa_faces[17] * truncicosa_ratio,
    icosa_faces[18] * truncicosa_ratio,
    icosa_faces[19] * truncicosa_ratio};

} // solids
} // model
