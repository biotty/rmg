//      © Christian Sommerfeldt Øien
//      All rights reserved

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
#include "render.hpp"

#include <cstdio>
#include <cstdarg>
#include <iostream>


    std::istream &
operator>>(std::istream & ist, xyz & p)
{
    ist >> p.x >> p.y >> p.z;
    return ist;
}

    std::istream &
operator>>(std::istream & ist, color & c)
{
    ist >> c.r >> c.g >> c.b;
    return ist;
}

    std::istream &
operator>>(std::istream & ist, compact_color & cc)
{
    color c;
    ist >> c;
    cc = z_filter(c);
    return ist;
}


    [[ noreturn ]] void
fail(const char * fmt, ...)
{
    std::va_list ap;
    va_start(ap, fmt);
    std::vfprintf(stderr, fmt, ap);
    va_end(ap);
    std::exit(EXIT_FAILURE);
}

    observer
get_observer()
{
    observer ret;
    std::cin >> ret.eye;
    std::cin >> ret.view;
    std::cin >> ret.column_direction;
    direct_row(&ret);
    return ret;
}

    void *
get_object(std::string name,
        object_intersection * fi, object_normal * fn)
{
    if (name == "plane" || name == "-plane") {
        auto plane_ = alloc<plane>();
        std::cin >> plane_->at;
        std::cin >> plane_->normal;
        normalize(&plane_->normal);
        *fi = plane_intersection;
        *fn = plane_normal;
        if (name[0] == '-') {
            scale(&plane_->normal, -1);
        }
        return plane_;
    }

    if (name == "sphere" || name == "-sphere") {
        real radius;
        auto sphere_ = alloc<sphere>();
        std::cin >> sphere_->center;
        std::cin >> radius;
        sphere_->sq_radius = square(radius);
        if (name[0] == '-') {
            *fi = _sphere_intersection;
            *fn = _sphere_normal;
        } else {
            *fi = sphere_intersection;
            *fn = sphere_normal;
        }
        return sphere_;
    }

    if (name == "cylinder" || name == "-cylinder") {
        real r;
        rotation_arg rota;
        point p;
        direction axis;
        std::cin >> p;
        std::cin >> axis;
        spherical_arg(axis, &r, &rota);
        auto cylinder_ = alloc<cylinder>();
        *cylinder_ = cylinder{
            direction{-p.x, -p.y, -p.z},
            (float)square(r), rota};
        if (name[0] == '-') {
            *fi = _cylinder_intersection;
            *fn = _cylinder_normal;
        } else {
            *fi = cylinder_intersection;
            *fn = cylinder_normal;
        }
        return cylinder_;
    }

    if (name == "cone" || name == "-cone") {
        real r;
        rotation_arg rota;
        point apex;
        direction axis;
        std::cin >> apex;
        std::cin >> axis;
        spherical_arg(axis, &r, &rota);
        auto cone_ = alloc<cone>();
        *cone_ = cone{
            direction{-apex.x, -apex.y, -apex.z},
            1/(float)r, rota};
        if (name[0] == '-') {
            *fi = _cone_intersection;
            *fn = _cone_normal;
        } else {
            *fi = cone_intersection;
            *fn = cone_normal;
        }
        return cone_;
    }

    if (name == "parabol" || name == "-parabol") {
        real r_half;
        rotation_arg rota;
        point vertex;
        direction focus;
        std::cin >> vertex;
        std::cin >> focus;
        spherical_arg(focus, &r_half, &rota);
        auto parabol_ = alloc<parabol>();
        *parabol_ = parabol{
            distance_vector(vertex, point_from_origo(focus)),
            2 *(float) r_half, rota};
        if (name[0] == '-') {
            *fi = _parabol_intersection;
            *fn = _parabol_normal;
        } else {
            *fi = parabol_intersection;
            *fn = parabol_normal;
        }
        return parabol_;
    }

    if (name == "hyperbol" || name == "-hyperbol") {
        real r;
        rotation_arg rota;
        point center;
        direction axis;
        real vertex;
        std::cin >> center;
        std::cin >> axis;
        std::cin >> vertex;
        spherical_arg(axis, &r, &rota);
        auto hyperbol_ = alloc<hyperbol>();
        *hyperbol_ = hyperbol{
            direction{-center.x, -center.y, -center.z},
            rota, 1/(float)r, 1/(float)vertex};
        if (name[0] == '-') {
            *fi = _hyperbol_intersection;
            *fn = _hyperbol_normal;
        } else {
            *fi = hyperbol_intersection;
            *fn = hyperbol_normal;
        }
        return hyperbol_;
    }

    if (name == "saddle" || name == "-saddle") {
        real r;
        rotation_arg rota;
        point center;
        direction axis;
        real v;
        std::cin >> center;
        std::cin >> axis;
        std::cin >> v;
        spherical_arg(axis, &r, &rota);
        auto saddle_ = alloc<saddle>();
        *saddle_ = saddle{
            direction{-center.x, -center.y, -center.z},
            rota, (float)v, 1/(float)r};
        *fi = saddle_intersection;
        *fn = saddle_normal;
        if (name[0] == '-') {
            saddle_->h *= -1;
            saddle_->v += (float)REAL_PI / 2;
        }
        return saddle_;
    }

    fail("scene object type \"%s\"?\n", name.c_str());
}

    void *
get_member(object_intersection * fi, object_normal * fn, void *)
{
    std::string name;
    std::cin >> name;
    return get_object(name, fi, fn);
}

    object_optics
get_object_optics()
{
    object_optics ret;
    std::cin >> ret.reflection_filter;
    std::cin >> ret.absorption_filter;
    std::cin >> ret.refraction_index;
    std::cin >> ret.refraction_filter;
    std::cin >> ret.passthrough_filter;
    return ret;
}

    texture_application
get_texture_application()
{
    texture_application ret;
    std::cin >> ret.reflection_factor;
    std::cin >> ret.absorption_factor;
    std::cin >> ret.refraction_factor;
    return ret;
}

    void *
get_decoration(std::string name, object_decoration * df)
{
    direction n;
    real w;
    point o;
    std::string path;

    if (name == "normal") {
        std::cin >> n >> w >> path;
        return normal_texture_mapping(df, n, w, path.c_str(),
                get_texture_application());
    }

    void * (* mapping)(object_decoration * df, direction n, real w,
            point o, const char * path, texture_application a) = nullptr;
    if (name == "planar") mapping = planar_texture_mapping;
    if (name == "planar1") mapping = planar1_texture_mapping;
    if (name == "relative") mapping = relative_texture_mapping;
    if (name == "axial") mapping = axial_texture_mapping;
    if (name == "axial1") mapping = axial1_texture_mapping;
    if (mapping) {
        std::cin >> n >> w >> o >> path;
        return mapping(df, n, w, o, path.c_str(),
                get_texture_application());
    }

    if (name == "checkers") {
        int q;
        compact_color reflection, absorption, refraction;
        std::cin >> n >> w >> o >> q >> reflection
            >> absorption >> refraction;
        return checkers_mapping(df, n, w, o, q, reflection,
                absorption, refraction);
    }

    fail("decoration \"%s\"?", name.c_str());
}

    void
get_scene_object(int i, world & world_)
{
    object_intersection fi;
    object_normal fn;
    void * a;

    std::string name;
    std::cin >> name;
    if (name == "x") {
        int n_members;
        std::cin >> n_members;
        a = make_inter(&fi, &fn, n_members, get_member, nullptr);
    } else {
        a = get_object(name, &fi, &fn);
    }

    object_decoration df = nullptr;
    void * d = nullptr;
    std::string alt;
    std::cin >> alt;
    if (alt != "optics") {
        d = get_decoration(alt, &df);
        if ( ! d) fail("decoration [%d] error\n", i);
        std::cin >> alt;
        if (alt != "optics") fail("optics [%d] error\n", i);
    }
    auto o = scene_object{ fi, fn, a, get_object_optics(), df, d };
    if (name == "x") world_.inter_args.push_back(const_cast<void *>(o.object_arg));
    else world_.object_args.push_back(const_cast<void *>(o.object_arg));
    if (o.decoration_arg) world_.decoration_args.push_back(const_cast<void *>(o.decoration_arg));
    world_.scene_.push_back(o);
}


    scene_sky
get_scene_sky()
{
    std::string name;
    std::cin >> name;
    if (name == "rgb") {
        return rgb_sky;
    } else if (name == "hsv") {
        return hsv_sky;
    } else if (name == "color") {
        std::cin >> sky_color[0];
        std::cin >> sky_color[1];
        std::cin >> sky_color[2];
        return color_sky;
    }

    sky_photo = photo_create(name.c_str());
    if ( ! sky_photo) fail("could not load sky '%s'\n", name.c_str());
    return photo_sky;
}

    void
get_spots(spots & _)
{
    unsigned light_spot_count;
    std::cin >> light_spot_count;
    for (size_t i = 0; i < light_spot_count; i++) {
        light_spot s;
        std::cin >> s.spot;
        std::cin >> s.light;
        _.push_back(s);
    }
}

    int
main(int argc, char *argv[])
{
    if (argc < 2) fail("dimention argument missing\n");
    char * out_path = (argc == 2) ? nullptr : argv[2];
    int width;
    int height;
    if (2 != sscanf(argv[1], "%dx%d", &width, &height))
        fail("dimention argument '%s' not on 'WxH' format\n", argv[1]);

    world world_{nullptr, delete_inter, delete_decoration};
    observer obs = get_observer();

    unsigned scene_object_count;
    std::cin >> scene_object_count;
    for (size_t i = 0; i < scene_object_count; i++) {
        assert(i == world_.scene_.size());
        get_scene_object(i, world_);
    }

    world_.sky = get_scene_sky();
    get_spots(world_.spots_);

    char c;
    while (std::cin >> c) {
        if ( ! std::isspace(c)) fail("non-space trailer. got '%c'\n", c);
    } // wait till we get end-of-file (polite to not break the pipe)
    
    int n_workers = getenv("GUN1") ? 1 : 0/* n.cores */;
    render(out_path, width, height, obs, world_, n_workers);
}
