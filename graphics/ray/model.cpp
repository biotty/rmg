//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "model.hpp"
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
#include "observer.h"

#include <cmath>
#include <type_traits>

namespace model {

    point
point_cast(direction d)
{
    return { d.x, d.y, d.z };
}

    direction
direction_cast(point p)
{
    return { p.x, p.y, p.z };
}

    void
mul_(point & p, double factor)
{
    p.x *= factor;
    p.y *= factor;
    p.z *= factor;
}

    void
mov_(point & p, direction offset)
{
    p.x += offset.x;
    p.y += offset.y;
    p.z += offset.z;
}

    void
mul_(direction & d, const double m[9])
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
rot_(direction & d, direction axis, double angle)
{
    const direction & u = axis;
    const double c = std::cos(angle);
    const double l_c = 1 - c;
    const double s = std::sin(angle);
    const double m[9] = {
        u.x*u.x + (1 - u.x*u.x)*c, u.x*u.y*l_c + u.z*s, u.x*u.z*l_c - u.y*s,
        u.x*u.y*l_c - u.z*s, u.y*u.y+(1 - u.y*u.y)*c, u.y*u.z*l_c + u.x*s,
        u.x*u.z*l_c + u.y*s, u.y*u.z*l_c - u.x*s,  u.z*u.z + (1 - u.z*u.z)*c
    };
    mul_(d, m);
}

    void
neg_(direction & d)
{
    d.x = -d.x;
    d.y = -d.y;
    d.z = -d.z;
}

    void
rot_(point & p, point at, direction axis, double angle)
{
    direction d = direction_cast(at);
    direction e = d;
    neg_(e);
    mov_(p, e);
    direction q = direction_cast(p);
    rot_(q, axis, angle);
    p = point_cast(q);
    mov_(p, d);
}

void plane::_mul(double) {}
void plane::_mov(direction offset) { mov_(p, offset); }
void plane::_rot(point at, direction axis, double angle)
{
    rot_(p, at, axis, angle);
    rot_(d, axis, angle);
}

void sphere::_mul(double factor) { r *= factor; }
void sphere::_mov(direction offset) { mov_(p, offset); }
void sphere::_rot(point, direction, double) {}

void common_geometric::_mul(double factor) { r *= factor; }
void common_geometric::_mov(direction offset) { mov_(p, offset); }
void common_geometric::_rot(point at, direction axis, double angle)
{
    rot_(p, at, axis, angle);
    rot_(d, axis, angle);
}

void hyperbol::_mul(double factor) {
    r *= factor;
    h *= factor;
}

void saddle::_mul(double factor) { h *= factor; }
void saddle::_mov(direction offset) { mov_(p, offset); }
void saddle::_rot(point at, direction axis, double angle)
{
    rot_(p, at, axis, angle);
    rot_(d, axis, angle);
    rot_(x, axis, angle);
}

void mul_(shape & s, double factor)
{
    std::visit([factor](auto & arg) { arg._mul(factor); }, s);
}

void mov_(shape & s, direction offset)
{
    std::visit([offset](auto & arg) { arg._mov(offset); }, s);
}

void rot_(shape & s, point at, direction axis, double angle)
{
    std::visit([at, axis, angle](auto & arg) { arg._rot(at, axis, angle); }, s);
}

void angular::_mul(double factor) { r *= factor; }
void angular::_mov(direction) {}
void angular::_rot(point, direction axis, double angle) { rot_(d, axis, angle); }

    void
mul_(texture & s, double factor)
{
    std::visit([factor](auto & arg) { arg._mul(factor); }, s);
}

    void
mov_(texture & s, direction offset)
{
    std::visit([offset](auto & arg) { arg._mov(offset); }, s);
}

    void
rot_(texture & s, point at, direction axis, double angle)
{
    std::visit([at, axis, angle](auto & arg) { arg._rot(at, axis, angle); }, s);
}

    void
mul_(inter & s, double factor) {
    for (auto & o : s)
        std::visit([factor](auto & arg) { arg._mul(factor); }, o);
}

    void
mov_(inter & s, direction offset) {
    for (auto & o : s)
        std::visit([offset](auto & arg) { arg._mov(offset); }, o);
}

    void
rot_(inter & s, point at, direction axis, double angle) {
    for (auto & o : s)
        std::visit([at, axis, angle](auto & arg) { arg._rot(at, axis, angle); }, o);
}

    object
object::mul(double factor)
{
    object ret = *this;
    std::visit([factor](auto & arg) { mul_(arg, factor); }, ret.s);
    if (ret.u)
        mul_(*ret.u, factor);
    return ret;
}

    object
object::mov(direction offset)
{
    object ret = *this;
    std::visit([offset](auto & arg) { mov_(arg, offset); }, ret.s);
    if (ret.u)
        mov_(*ret.u, offset);
    return ret;
}

    object
object::rot(point at, direction axis, double angle)
{
    object ret = *this;
    std::visit([at, axis, angle](auto & arg) { rot_(arg, at, axis, angle); }, ret.s);
    if (ret.u)
        rot_(*ret.u, at, axis, angle);
    return ret;
}

}

namespace {

    point
make(model::point p)
{ return { p.x, p.y, p.z }; }

    direction
make(model::direction d)
{ return { d.x, d.y, d.z }; }

    direction
make_norm(model::direction d)
{
    direction n = make(d);
    normalize(&n);
    return n;
}

    rotation_arg
make_tilt(model::direction d)
{
    rotation_arg rota;
    real ignore_r;
    spherical_arg(make(d), &ignore_r, &rota);
    return rota;
}

    void *
make(model::plane pl, object_intersection * fi, object_normal * fn, bool inv)
{
        auto plane_ = alloc<plane>();
        plane_->at = make(pl.p);
        plane_->normal = make_norm(pl.d);
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
    sphere_->center = make(sp.p);
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
        distance_vector(make(cy.p), origo),
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
        distance_vector(make(co.p), origo),
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
    *parabol_ = parabol{
        distance_vector(make(pa.p), point_from_origo(make(pa.d))),
            2 *(float) pa.r, make_tilt(pa.d)};
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
        distance_vector(make(hy.p), origo),
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
    rotation_arg rota = make_tilt(sa.d);
    direction a1 = inverse_rotation(make(sa.x), rota);
    auto saddle_ = alloc<saddle>();
    *saddle_ = saddle{
        distance_vector(make(sa.p), origo), rota,
            (float)ratan(a1.y, a1.x), 1 /(float) sa.h};
    *fi = saddle_intersection;
    *fn = saddle_normal;
    if (inv) {
        saddle_->h *= -1;
        saddle_->v += (float)REAL_PI / 2;
    }
    return saddle_;
}

    void *
make(model::shape sh, object_intersection * fi, object_normal * fn, world * world_)
{
    // note: different overload(/name) prevents that if function for variant-
    //       alternative is omitted by mistake then we get compile error
    //       instead of silent setup for infinite recursion by overload-
    //       match via implicit conversion by variant constructor
    void * ret;
    std::visit([&ret, fi, fn, world_](auto arg) {
            bool inv = std::is_base_of<model::inv_shape, decltype(arg)>::value;
            ret = make(arg, fi, fn, inv);
            if (world_) world_->object_args.push_back(ret);
            }, sh);
    return ret;
}

    void *
member_get(object_intersection * fi, object_normal * fn, void * state)
{
    auto pp = static_cast<model::shape **>(state);
    return make(*(*pp)++, fi, fn, nullptr);
}

    void *
make(model::inter in, object_intersection * fi, object_normal * fn, world * world_)
{
    assert(world_);
    model::shape * ptr = &in[0];
    void * ret = make_inter(fi, fn, in.size(), member_get, &ptr);
    world_->inter_args.push_back(ret);
    return ret;
}

    color
make(model::color c)
{ return { c.r, c.g, c.b }; }

    texture_application
make(model::surface s)
{
    return {
        make(s.reflection),
        make(s.absorption),
        make(s.refraction)
    };
}

    void *
make(model::angular tx, object_decoration * df)
{ return normal_texture_mapping(df, make(tx.d), tx.r, tx.n, make(tx.s)); }

    void *
make(model::planar tx, object_decoration * df)
{ return planar_texture_mapping(df, make(tx.d), tx.r, make(tx.p), tx.n, make(tx.s)); }

    void *
make(model::planar1 tx, object_decoration * df)
{ return planar1_texture_mapping(df, make(tx.d), tx.r, make(tx.p), tx.n, make(tx.s)); }

    void *
make(model::relative tx, object_decoration * df)
{ return relative_texture_mapping(df, make(tx.d), tx.r, make(tx.p), tx.n, make(tx.s)); }

    void *
make(model::axial tx, object_decoration * df)
{ return axial_texture_mapping(df, make(tx.d), tx.r, make(tx.p), tx.n, make(tx.s)); }

    void *
make(model::axial1 tx, object_decoration * df)
{ return axial1_texture_mapping(df, make(tx.d), tx.r, make(tx.p), tx.n, make(tx.s)); }

    compact_color
make_c(model::color c)
{ return z_filter(make(c)); }

    void *
make(model::checkers tx, object_decoration * df)
{
    compact_color reflection = make_c(tx.s.reflection);
    compact_color absorption = make_c(tx.s.absorption);
    compact_color refraction = make_c(tx.s.refraction);
    return checkers_mapping(df, make(tx.d), tx.r, make(tx.p), tx.q,
        reflection, absorption, refraction);
}

    void *
make(model::texture tx, object_decoration * df, world & world_)
{
    void * ret;
    std::visit([&ret, df, &world_](auto arg){
            ret = make(arg, df);
            world_.decoration_args.push_back(ret);
            }, tx);
    return ret;
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
make(model::object obj, world & world_)
{
    object_intersection fi;
    object_normal fn;
    void * a;

    std::visit([&](auto arg){ a = make(arg, &fi, &fn, &world_); }, obj.s);

    object_decoration fd = nullptr;
    auto d = obj.u ? make(*obj.u, &fd, world_) : nullptr;
    auto o = scene_object{ fi, fn, a, make(obj.o), fd, d };
    world_.scene_.push_back(o);
}

    observer
make(model::observer o)
{
    observer ret;
    ret.eye = make(o.e);
    ret.view = make(o.c);
    ret.column_direction = make(o.x);
    direct_row(&ret);
    return ret;
}

    std::pair<observer, world>
make(model::world w)
{
    std::pair<observer, world> ret{
        make(w.obs), world{w.sky, delete_inter, delete_decoration}
    };
    for (auto o : w.s) make(o, ret.second);
    for (auto s : w.ls)
        ret.second.spots_.push_back({make(s.p), make(s.c)});
    return ret;
}

}

namespace model {

    void
render(
    const char * path, int width, int height,
    model::world w, unsigned n_threads)
{
    auto [obs, world_] = make(w);
    ::render(path, width, height, obs, world_, n_threads);
}

}
