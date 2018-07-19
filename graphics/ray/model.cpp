//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "model.hpp"
#include "plane.h"
#include "sphere.h"
#include "cylinder.h"
#include "cone.h"
#include "parabol.h"
#include "hyperbol.h"
#include "saddle.h"
#include "inter.h"
#include "mapping.h"
#include "observer.hpp"


    direction
make(model::direction d)
{
    direction n = { d.x, d.y, d.z };
    normalize(&n);
    return n;
}

    point
make(model::point p)
{ return { p.x, p.y, p.z }; }

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

    void *
make(model::plane pl, object_intersection * fi, object_normal * fn)
{
        auto plane_ = alloc<plane>();
        plane_->at = make(pl.p);
        plane_->normal = make(pl.d);
        normalize(&plane_->normal);
        *fi = plane_intersection;
        *fn = plane_normal;
        if (pl.inv) {
            scale(&plane_->normal, -1);
        }
        return plane_;
}

    void *
make(model::sphere sp, object_intersection * fi, object_normal * fn)
{
    auto sphere_ = alloc<sphere>();
    sphere_->center = make(sp.p);
    sphere_->sq_radius = square(sp.r);
    if (sp.inv) {
        *fi = _sphere_intersection;
        *fn = _sphere_normal;
    } else {
        *fi = sphere_intersection;
        *fn = sphere_normal;
    }
    return sphere_;
}

    void *
make(model::cylinder cy, object_intersection * fi, object_normal * fn)
{
    real ignore;
    rotation_arg rota;
    spherical_arg(make(cy.d), &ignore, &rota);
    auto cylinder_ = alloc<cylinder>();
    *cylinder_ = cylinder{
        distance_vector(make(cy.p), origo),
            (float)square(cy.r), rota};
    if (cy.inv) {
        *fi = _cylinder_intersection;
        *fn = _cylinder_normal;
    } else {
        *fi = cylinder_intersection;
        *fn = cylinder_normal;
    }
    return cylinder_;
}

    void *
make(model::cone co, object_intersection * fi, object_normal * fn)
{
    real ignore;
    rotation_arg rota;
    spherical_arg(make(co.d), &ignore, &rota);
    auto cone_ = alloc<cone>();
    *cone_ = cone{
        distance_vector(make(co.p), origo),
            1/(float)co.r, rota};
    if (co.inv) {
        *fi = _cone_intersection;
        *fn = _cone_normal;
    } else {
        *fi = cone_intersection;
        *fn = cone_normal;
    }
    return cone_;
}

    void *
make(model::parabol pa, object_intersection * fi, object_normal * fn)
{
    real ignore;
    rotation_arg rota;
    spherical_arg(make(pa.d), &ignore, &rota);
    auto parabol_ = alloc<parabol>();
    *parabol_ = parabol{
        distance_vector(make(pa.p), point_from_origo(make(pa.d))),
            2 *(float) pa.r, rota};
    if (pa.inv) {
        *fi = _parabol_intersection;
        *fn = _parabol_normal;
    } else {
        *fi = parabol_intersection;
        *fn = parabol_normal;
    }
    return parabol_;
}

    void *
make(model::hyperbol hy, object_intersection * fi, object_normal * fn)
{
    real ignore;
    rotation_arg rota;
    spherical_arg(make(hy.d), &ignore, &rota);
    auto hyperbol_ = alloc<hyperbol>();
    *hyperbol_ = hyperbol{
        distance_vector(make(hy.p), origo),
            rota, 1/(float)hy.h, 1/(float)hy.r};
    if (hy.inv) {
        *fi = _hyperbol_intersection;
        *fn = _hyperbol_normal;
    } else {
        *fi = hyperbol_intersection;
        *fn = hyperbol_normal;
    }
    return hyperbol_;
}

    void *
make(model::saddle sa, object_intersection * fi, object_normal * fn)
{
    real ignore;
    rotation_arg rota;
    spherical_arg(make(sa.d), &ignore, &rota);
    direction n = inverse_rotation(make(sa.z), rota);
    auto saddle_ = alloc<saddle>();
    *saddle_ = saddle{
        distance_vector(make(sa.p), origo),
            rota, (float)ratan(n.y, n.x), 1 /(float) sa.h};
    *fi = saddle_intersection;
    *fn = saddle_normal;
    if (sa.inv) {
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
            ret = make(arg, fi, fn);
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
make_z(model::color c)
{ return z_filter(make(c)); }

    void *
make(model::checkers tx, object_decoration * df)
{
    compact_color reflection = make_z(tx.s.reflection);
    compact_color absorption = make_z(tx.s.absorption);
    compact_color refraction = make_z(tx.s.refraction);
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
        make_z(o.reflection_filter),
        make_z(o.absorption_filter),
        make_z(o.refraction_filter),
        make_z(o.passthrough_filter)
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

    void
make(model::spots s, world & world_)
{
    for (auto sp : s) {
        light_spot s{make(sp.p), make(sp.c)};
        world_.spots_.push_back(s);
    }
}

    world
make(model::world w)
{
    world ret{delete_inter, delete_decoration};
    make(w.ls, ret);
    ret.sky = w.sky;

    for (auto o : w.os) {
        make(o, ret);
    }
    return ret;
}
