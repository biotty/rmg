//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "inter.h"
#include "mapping.h"
#include "observer.h"
#include "trace.h"
#include "image.h"
#include "xmath.h"
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
    cc = z_color(c);
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

    if (name == "plane") {
        point at;
        std::cin >> at;
        ret.plane_.at_x = at.x;
        ret.plane_.at_y = at.y;
        ret.plane_.at_z = at.z;
        std::cin >> ret.plane_.normal;
        normalize(&ret.plane_.normal);
        *fi = plane_intersection;
        *fn = plane_normal;
        return ret;
    }

    if (name == "sphere" || name == "-sphere") {
        std::cin >> ret.sphere_.center;
        std::cin >> ret.sphere_.radius;
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
        real r, theta, phi;
        direction p;
        direction axis;
        std::cin >> p;
        std::cin >> axis;
        spherical(axis, &r, &theta, &phi);
        cylinder cylinder_ = {{-p.x, -p.y, -p.z},
            (float)square(r), (float)theta, (float)phi};
        ret.cylinder_ = cylinder_;
        if (name[0] == '-') {
            *fi = _cylinder_intersection;
            *fn = _cylinder_normal;
        } else {
            *fi = cylinder_intersection;
            *fn = cylinder_normal;
        }
        return ret;
    }

    if (name == "cone" || name == "-cone") {
        real r, theta, phi;
        direction apex;
        direction axis;
        std::cin >> apex;
        std::cin >> axis;
        spherical(axis, &r, &theta, &phi);
        cone cone_ = {{-apex.x, -apex.y, -apex.z},
            1/(float)r, (float)theta, (float)phi};
        ret.cone_ = cone_;
        if (name[0] == '-') {
            *fi = _cone_intersection;
            *fn = _cone_normal;
        } else {
            *fi = cone_intersection;
            *fn = cone_normal;
        }
        return ret;
    }

    fail("scene object type \"%s\"?\n", name.c_str());
}

    object_arg_union *
make_object(std::string name,
        object_intersection * fi, object_normal * fn)
{
    object_arg_union * arg
        = static_cast<object_arg_union *>(arg_alloc(sizeof *arg));
    *arg = get_object(name, fi, fn);
    return arg;
}

    object_arg_union
get_member(object_intersection * fi, object_normal * fn)
{
    std::string name;
    std::cin >> name;
    return get_object(name, fi, fn);
}

    object_optics
get_object_optics(real first)
{
    color c;
    c.r = first;
    std::cin >> c.g;
    std::cin >> c.b;
    object_optics ret;
    ret.reflection_filter = z_color(c);
    std::cin >> ret.absorption_filter;
    real r;
    std::cin >> r;
    if (r >= ULONG_MAX / 1e9) fail("refraction index overflow\n");
    ret.refraction_index_nano = static_cast<unsigned long>(r * 1e9);
    std::cin >> ret.refraction_filter;
    std::cin >> ret.passthrough_filter;
    return ret;
}

    texture_application
get_texture_application()
{
    texture_application ret;
    std::cin >> ret.x_wrap;
    std::cin >> ret.y_wrap;
    std::cin >> ret.reflection_factor;
    std::cin >> ret.absorption_factor;
    std::cin >> ret.refraction_factor;
    return ret;
}

    void *
get_decoration(std::string name, object_decoration * df)
{
    point o;
    direction n;
    std::string path;

    if (name == "directional") {
        std::cin >> n >> path;
        return directional_texture_mapping(df, n, path.c_str(),
                get_texture_application());
    }

    if (name == "positional") {
        std::cin >> n >> path;
        return positional_texture_mapping(df, n, path.c_str(),
                get_texture_application());
    }

    if (name == "relative") {
        std::cin >> n >> o >> path;
        return relative_texture_mapping(df, n, o, path.c_str(),
                get_texture_application());
    }

    if (name == "linear") {
        std::cin >> n >> o >> path;
        return linear_texture_mapping(df, n, o, path.c_str(),
                get_texture_application());
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
    int k;
    std::cin >> k;
    w->spot_count = k;
    w->spots = new light_spot[k];
    for (int x=0; x<k; x++) {
        light_spot s;
        std::cin >> s.spot;
        std::cin >> s.light;
        w->spots[x] = s;
    }
}

    void
produce_trace(world * w, observer * o, int width, int height,
        const char * out_path, bool report_status)
{
    image out = image_create(out_path, width, height);
    const real aspect_ratio = width /(real) height;
    for (int row = 0; row < height; row++) {
        for (int column = 0; column < width; column++) {
            ray ray_ = observer_ray(o, aspect_ratio,
                    column /(real) width,
                    row /(real) height);
            image_write(out, trace(ray_, w));
        }
        if (report_status) std::cerr << "\r" << row;
    }
    image_close(out);
}

    int
main(int argc, char *argv[])
{
    bool report_status = getenv("GUN_RS") != NULL;
    if (argc < 2) fail("dimention argument missing\n");
    char * out_path = (argc == 2) ? NULL : argv[2];
    int width;
    int height;
    if (2 != sscanf(argv[1], "%dx%d", &width, &height))
        fail("dimention argument '%s' not on 'WxH' format\n", argv[1]);

    observer obs = get_observer();
    int scene_object_count;
    int inter_count;
    int member_count;
    std::cin >> scene_object_count >> inter_count >> member_count;
    if (scene_object_count <= 0) fail("no scene objects\n");
    if (inter_count > scene_object_count) fail("intersecions count overflow\n");

    world world_ = { color_sky, NULL, 0,
        { scene_object_count, new scene_object[scene_object_count] }
    };
    int non_inter_count = scene_object_count - inter_count;
    init_arg_pool(non_inter_count, inter_count, member_count);
    void ** decoration_args = new void *[scene_object_count];
    int decoration_index = 0;

    for (int i = 0; i < world_.scene_.object_count; i++) {
        object_intersection fi;
        object_normal fn;
        std::string name;
        std::cin >> name;
        void * a = NULL;
        if (name == "x") {
            int n_members;
            std::cin >> n_members;
            a = make_inter(&fi, &fn, n_members, get_member);
        } else {
            a = make_object(name, &fi, &fn);
        }
        if (a == NULL) fail("object [%d] error\n", i);

        object_decoration df = NULL;
        void * d = NULL;
        std::string alt;
        std::cin >> alt;
        real r_;
        if (std::isalpha(alt[0])) {
            d = get_decoration(alt, &df);
            if (d == NULL) fail("decoration [%d] error\n", i);
            decoration_args[decoration_index ++ ] = d;
            std::cin >> r_;
        } else {
            const int n = std::sscanf(alt.c_str(), REAL_FMT, &r_);
            if (n != 1) fail("optics [%d] error\n", i);
        }
        scene_object o = { fi, fn, a, get_object_optics(r_), df, d };
        world_.scene_.objects[i] = o;
    }

    world_.sky = get_scene_sky();
    
    get_spots(&world_);

    char c;
    while (std::cin >> c) {
        if ( ! std::isspace(c)) fail("non-space trailer. got '%c'\n", c);
    } // wait till we get end-of-file (polite to not break the pipe)
    
    produce_trace(&world_, &obs, width, height, out_path, report_status);

    fini_arg_pool();
    while ( -- decoration_index >= 0)
        delete_texture_mapping(decoration_args[decoration_index]);
    delete [] decoration_args;
    delete [] world_.scene_.objects;
    delete [] world_.spots;
    if (report_status) std::cerr << "\n";
}
