//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "inter.h"
#include "mapping.h"
#include "observer.hpp"
#include <cstdio>
#include <cctype>
#include <cstdarg>
#include <climits>
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
    std::cin >> ret.row_direction;
    return ret;
}

    object_arg_union
get_object(std::string name,
        object_intersection * fi, object_normal * fn)
{
    object_arg_union ret;

    if (name == "plane" || name == "-plane") {
        std::cin >> ret.plane_.at;
        std::cin >> ret.plane_.normal;
        normalize(&ret.plane_.normal);
        *fi = plane_intersection;
        *fn = plane_normal;
        if (name[0] == '-') {
            scale(&ret.plane_.normal, -1);
        }
        return ret;
    }

    if (name == "sphere" || name == "-sphere") {
        real radius;
        std::cin >> ret.sphere_.center;
        std::cin >> radius;
        ret.sphere_.sq_radius = square(radius);
        if (name[0] == '-') {
            *fi = _sphere_intersection;
            *fn = _sphere_normal;
        } else {
            *fi = sphere_intersection;
            *fn = sphere_normal;
        }
        return ret;
    }

    if (name == "cylinder" || name == "-cylinder") {
        real r;
        rotation_arg rota;
        point p;
        direction axis;
        std::cin >> p;
        std::cin >> axis;
        spherical_arg(axis, &r, &rota);
        cylinder cylinder_ = {{-p.x, -p.y, -p.z},
            (float)square(r), rota};
        if (name[0] == '-') {
            *fi = _cylinder_intersection;
            *fn = _cylinder_normal;
        } else {
            *fi = cylinder_intersection;
            *fn = cylinder_normal;
        }
        ret.cylinder_ = cylinder_;
        return ret;
    }

    if (name == "cone" || name == "-cone") {
        real r;
        rotation_arg rota;
        point apex;
        direction axis;
        std::cin >> apex;
        std::cin >> axis;
        spherical_arg(axis, &r, &rota);
        cone cone_ = {{-apex.x, -apex.y, -apex.z},
            1/(float)r, rota};
        if (name[0] == '-') {
            *fi = _cone_intersection;
            *fn = _cone_normal;
        } else {
            *fi = cone_intersection;
            *fn = cone_normal;
        }
        ret.cone_ = cone_;
        return ret;
    }

    if (name == "parabol" || name == "-parabol") {
        real r_half;
        rotation_arg rota;
        point vertex;
        direction focus;
        std::cin >> vertex;
        std::cin >> focus;
        spherical_arg(focus, &r_half, &rota);
        parabol parabol_ = {
            distance_vector(vertex, point_from_origo(focus)),
            2 *(float) r_half, rota};
        if (name[0] == '-') {
            *fi = _parabol_intersection;
            *fn = _parabol_normal;
        } else {
            *fi = parabol_intersection;
            *fn = parabol_normal;
        }
        ret.parabol_ = parabol_;
        return ret;
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
        hyperbol hyperbol_ = {{-center.x, -center.y, -center.z},
            rota, 1/(float)r, 1/(float)vertex};
        if (name[0] == '-') {
            *fi = _hyperbol_intersection;
            *fn = _hyperbol_normal;
        } else {
            *fi = hyperbol_intersection;
            *fn = hyperbol_normal;
        }
        ret.hyperbol_ = hyperbol_;
        return ret;
    }

    if (name == "saddle" || name == "-saddle") {
        real r;
        rotation_arg rota;
        point center;
        direction axis;
        real v, x, y;
        std::cin >> center;
        std::cin >> axis;
        std::cin >> v;
        std::cin >> x;
        std::cin >> y;
        spherical_arg(axis, &r, &rota);
        saddle saddle_ = {{-center.x, -center.y, -center.z},
            rota, (float)v, {1/(float)x, 1/(float)y, 1/(float)r }};
        *fi = saddle_intersection;
        *fn = saddle_normal;
        if (name[0] == '-') {
            saddle_.scale.z *= -1;
            saddle_.v += (float)REAL_PI / 2;
            const float x = saddle_.scale.x;
            saddle_.scale.x = saddle_.scale.y;
            saddle_.scale.y = x;
        }
        ret.saddle_ = saddle_;
        return ret;
    }

    fail("scene object type \"%s\"?\n", name.c_str());
}

    object_arg_union
get_member(object_intersection * fi, object_normal * fn)
{
    std::string name;
    std::cin >> name;
    return get_object(name, fi, fn);
}

    object_optics
get_object_optics()
{
    color c;
    std::cin >> c.r;
    std::cin >> c.g;
    std::cin >> c.b;
    object_optics ret;
    ret.reflection_filter = z_filter(c);
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

    scene_sky
get_scene_sky()
{
    sky_color = {1, 1, 1};
    std::string name;
    std::cin >> name;
    if (name == "rgb") {
        return rgb_sky;
    } else if (name == "hsv") {
        return hsv_sky;
    } else if (name == "color") {
        std::cin >> sky_color;
        return color_sky;
    }

    sky_photo = photo_create(name.c_str());
    if ( ! sky_photo) fail("could not load sky '%s'\n", name.c_str());
    return photo_sky;
}

    void
get_spots(world * w)
{
    unsigned light_spot_count;
    std::cin >> light_spot_count;
    for (size_t i = 0; i < light_spot_count; i++) {
        assert(i == w->spots_.size());
        light_spot s;
        std::cin >> s.spot;
        std::cin >> s.light;
        w->spots_.push_back(s);
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

    observer obs = get_observer();
    unsigned scene_object_count;
    unsigned inter_count;
    std::cin >> scene_object_count >> inter_count;
    if (inter_count > scene_object_count) fail("impossible inter count\n");

    world world_;
    std::vector<object_arg_union> non_inter_args;
    std::vector<void *> inter_args;
    std::vector<void *> decoration_args;

    non_inter_args.reserve(scene_object_count - inter_count);  // no realloc

    for (size_t i = 0; i < scene_object_count; i++) {
        assert(i == world_.scene_.size());
        object_intersection fi;
        object_normal fn;
        std::string name;
        std::cin >> name;
        void * a = nullptr;
        if (name == "x") {
            int n_members;
            std::cin >> n_members;
            a = make_inter(&fi, &fn, n_members, get_member);
            if ( ! a) fail("object [%d] error\n", i);
            inter_args.push_back(a);
        } else {
            non_inter_args.push_back(get_object(name, &fi, &fn));
            a = &non_inter_args.back();
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
            decoration_args.push_back(d);
        }
        scene_object o = { fi, fn, a, get_object_optics(), df, d };
        world_.scene_.push_back(o);
    }

    world_.sky = get_scene_sky();
    
    get_spots(&world_);

    char c;
    while (std::cin >> c) {
        if ( ! std::isspace(c)) fail("non-space trailer. got '%c'\n", c);
    } // wait till we get end-of-file (polite to not break the pipe)
    
    int n_workers = getenv("GUN_SINGLE") ? 1 : 0/* n.cores */;
    produce_trace(out_path, width, height, world_, obs, n_workers);

    for (void * a : inter_args) free(a);
    for (void * d : decoration_args)
        delete_decoration(d);
}
