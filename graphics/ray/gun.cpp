//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "inter.h"
#include "mapping.h"
#include "observer.h"
#include "trace.h"
#include "image.h"
#include "xmath.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

    static void
fail(const char * fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}

typedef struct {
    char buf[256];
} bufstr_256;

#define SCAN_DEFINITION(T, N, F, R) static T g ## N (void) \
{ T N; if (1 != scanf(F, R)) fail("%s\n", F); return N; }
SCAN_DEFINITION(int, i, "%d", &i)
SCAN_DEFINITION(real, r, REAL_FMT, &r)
SCAN_DEFINITION(bufstr_256, s, "%s", s.buf)

    observer
get_observer()
{
    observer r = {
        .eye = {gr(), gr(), gr()},
        .view = {gr(), gr(), gr()},
        .column_direction = {gr(), gr(), gr()},
        .row_direction = {gr(), gr(), gr()},
    };
    return r;
}

    object_arg_union
get_object(const char * object_class,
        object_intersection * fi, object_normal * fn)
{
    object_arg_union a;
    if (0 == strcmp(object_class, "plane")) {
        plane plane_ = {
            .at_surface = {gr(), gr(), gr()},
            .normal = {gr(), gr(), gr()}};
        normalize(&plane_.normal);
        a.plane_ = plane_;
        *fi = plane_intersection;
        *fn = plane_normal;
    } else if (0 == strcmp(object_class, "sphere")
            || 0 == strcmp(object_class, "-sphere")) {
        sphere sphere_ = {
            .center = {gr(), gr(), gr()},
            .radius = gr()};
        a.sphere_ = sphere_;
        if (*object_class != '-') {
            *fi = sphere_intersection;
            *fn = sphere_normal;
        } else {
            *fi = _sphere_intersection;
            *fn = _sphere_normal;
        }
    } else if (0 == strcmp(object_class, "cylinder")
            || 0 == strcmp(object_class, "-cylinder")) {
        real r, theta, phi;
        direction p = {gr(), gr(), gr()};
        direction axis = {gr(), gr(), gr()};
        spherical(axis, &r, &theta, &phi);
        cylinder cylinder_ = {{-p.x, -p.y, -p.z}, square(r), theta, phi};
        a.cylinder_ = cylinder_;
        if (*object_class != '-') {
            *fi = cylinder_intersection;
            *fn = cylinder_normal;
        } else {
            *fi = _cylinder_intersection;
            *fn = _cylinder_normal;
        }
    } else if (0 == strcmp(object_class, "cone")
            || 0 == strcmp(object_class, "-cone")) {
        real r, theta, phi;
        direction apex = {gr(), gr(), gr()};
        direction axis = {gr(), gr(), gr()};
        spherical(axis, &r, &theta, &phi);
        cone cone_ = {{-apex.x, -apex.y, -apex.z}, 1/r, theta, phi};
        a.cone_ = cone_;
        if (*object_class != '-') {
            *fi = cone_intersection;
            *fn = cone_normal;
        } else {
            *fi = _cone_intersection;
            *fn = _cone_normal;
        }
    } else {
        fail("object class \"%s\"?\n", object_class);
    }

    return a;
}

    object_arg_union *
new_object(const char * object_class,
        object_intersection * fi, object_normal * fn)
{
    object_arg_union * arg
        = static_cast<object_arg_union *>(arg_alloc(sizeof *arg));
    *arg = get_object(object_class, fi, fn);
    return arg;
}

    object_arg_union
get_member(object_intersection * fi, object_normal * fn)
{
    return get_object(gs().buf, fi, fn);
}

    compact_color
make_compact_color(real r, real g, real b)
{
    color c = { r, g, b };
    return z_color(c);
}

    object_optics
get_object_optics(real alt_gr)
{
    object_optics ret;
    ret.reflection_filter = make_compact_color(alt_gr, gr(), gr());
    ret.absorption_filter = make_compact_color(gr(), gr(), gr());
    ret.refraction_index_micro = static_cast<unsigned>(gr() * 1000000);
    ret.refraction_filter = make_compact_color(gr(), gr(), gr());
    ret.passthrough_filter = make_compact_color(gr(), gr(), gr());
    return ret;
}

    texture_application
get_texture_application()
{
    texture_application ret;
    ret.x_wrap = gr();
    ret.y_wrap = gr();
    ret.reflection_factor = (color){gr(), gr(), gr()};
    ret.absorption_factor = (color){gr(), gr(), gr()};
    ret.refraction_factor = (color){gr(), gr(), gr()};
    return ret;
}

    void *
get_decoration(const char * deco_name, object_decoration * df)
{
    if (strcmp(deco_name, "directional") == 0) {
        direction n = {gr(), gr(), gr()};
        bufstr_256 path = gs();
        texture_application a = get_texture_application();
        return directional_texture_mapping(df, n, path.buf, a);
    } else if (strcmp(deco_name, "positional") == 0) {
        direction n = {gr(), gr(), gr()};
        bufstr_256 path = gs();
        texture_application a = get_texture_application();
        return positional_texture_mapping(df, n, path.buf, a);
    } else if (strcmp(deco_name, "relative") == 0) {
        direction n = {gr(), gr(), gr()};
        point o = {gr(), gr(), gr()};
        bufstr_256 path = gs();
        texture_application a = get_texture_application();
        return relative_texture_mapping(df, n, o, path.buf, a);
    } else if (strcmp(deco_name, "linear") == 0) {
        direction n = {gr(), gr(), gr()};
        point o = {gr(), gr(), gr()};
        bufstr_256 path = gs();
        texture_application a = get_texture_application();
        return linear_texture_mapping(df, n, o, path.buf, a);
    } else {
        fail("decoration \"%s\"?", deco_name);
    }
    return NULL;
}

    scene_sky
get_scene_sky()
{
    sky_color = {1, 1, 1};
    bufstr_256 sky_bs = gs();
    char * sky_name = sky_bs.buf;
    if (strcmp(sky_name, "rgb") == 0) {
        return rgb_sky;
    } else if (strcmp(sky_name, "color") == 0) {
        color sky = { gr(), gr(), gr() };
        sky_color = sky;
        return color_sky;
    }

    sky_photo = photo_create(sky_name);
    if ( ! sky_photo) fail("could not load sky '%s'\n", sky_name);
    return photo_sky;
}

    void
get_spots(world * w)
{
    const int k = gi();
    w->spot_count = k;
    w->spots = new light_spot[k];
    for (int x=0; x<k; x++) {
        light_spot s = {
            {gr(), gr(), gr()},
            {gr(), gr(), gr()}
        };
        w->spots[x] = s;
    }
}

    void
produce_trace(world * w, observer * o, int width, int height,
        const char * out_path, bool report_status)
{
    image out = image_create(out_path, width, height);
    if (report_status)
        fprintf(stderr, "Tracing %dx%d of Observer\ny", width, height);
    const real aspect_ratio = width /(real) height;
    for (int row = 0; row < height; row++) {
        for (int column = 0; column < width; column++) {
            ray ray_ = observer_ray(o, aspect_ratio,
                    column /(real) width,
                    row /(real) height);
            image_write(out, trace(ray_, w));
        }
        if (report_status) fprintf(stderr, "\r%d", row);
    }
    image_close(out);
}

    int
main(int argc, char *argv[])
{
    const bool report_status = getenv("GUN_RS") != NULL;
    if (argc < 2) fail("dimention argument missing\n");
    const char * dim_w = argv[1];
    const char * dim_h = strchr(dim_w, 'x');
    if ( ! dim_h) fail("dimention argument 'x' separator missing\n");
    ++dim_h;
    int width = atoi(dim_w);
    int height = atoi(dim_h);
    const char * const out_path = (argc == 2) ? NULL : argv[2];

    observer obs = get_observer();
    int scene_object_count = gi();
    int inter_count = gi();
    int member_count = gi();
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
        bufstr_256 name_bs = gs();
        char * name = name_bs.buf;
        void * a = (strcmp(name, "x"))
            ? new_object(name, &fi, &fn)
            : new_inter(&fi, &fn, gi(), get_member);
        if (!a) fail("object [%d] error\n", i);

        object_decoration df = NULL;
        void * d = NULL;
        bufstr_256 alt_bs = gs();
        const char * alt = alt_bs.buf;
        real gr_;
        if (isalpha(alt[0])) {
            d = get_decoration(alt, &df);
            if ( ! d) fail("decoration [%d] error\n", i);
            decoration_args[decoration_index ++ ] = d;
            gr_ = gr();
        } else {
            const int n = sscanf(alt, REAL_FMT, &gr_);
            if (n != 1) fail("optics [%d] error\n", i);
        }
        scene_object o = { fi, fn, a, get_object_optics(gr_), df, d };
        world_.scene_.objects[i] = o;
    }

    world_.sky = get_scene_sky();
    
    get_spots(&world_);
    
    while (fgetc(stdin) != EOF)
        ; // wait till we get end-of-file (polite to not break the pipe)
    
    produce_trace(&world_, &obs, width, height, out_path, report_status);

    fini_arg_pool();
    while ( -- decoration_index >= 0)
        delete_texture_mapping(decoration_args[decoration_index]);
    delete [] decoration_args;
    delete [] world_.scene_.objects;
    delete [] world_.spots;
    if (report_status) fprintf(stderr, "\n");
}
