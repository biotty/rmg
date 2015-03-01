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

    void *
new_object(const char * object_class,
        object_intersection * fi, object_normal * fn)
{
    object_arg_union * object_arg = new_object_arg();
    if (0 == strcmp(object_class, "plane")) {
        plane plane_ = {
            .at_surface = {gr(), gr(), gr()},
            .normal = {gr(), gr(), gr()}};
        normalize(&plane_.normal);
        object_arg->plane_ = plane_;
        *fi = plane_intersection;
        *fn = plane_normal;
    } else if (0 == strcmp(object_class, "sphere")
            || 0 == strcmp(object_class, "-sphere")) {
        sphere sphere_ = {
            .center = {gr(), gr(), gr()},
            .radius = gr()};
        object_arg->sphere_ = sphere_;
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
        object_arg->cylinder_ = cylinder_;
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
        object_arg->cone_ = cone_;
        if (*object_class != '-') {
            *fi = cone_intersection;
            *fn = cone_normal;
        } else {
            *fi = _cone_intersection;
            *fn = _cone_normal;
        }
    } else {
        free(object_arg);
        fail("object class \"%s\"?\n", object_class);
    }
    return object_arg;
}

    texture_application
get_texture_application()
{
    return (texture_application){
        .x_wrap = gr(),
        .y_wrap = gr(),
        .reflection_factor = (color){gr(), gr(), gr()},
        .absorption_factor = (color){gr(), gr(), gr()},
        .refraction_factor = (color){gr(), gr(), gr()},
    };
}

    void *
new_decoration(const char * deco_name, object_decoration * df)
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

    void *
new_member(object_intersection * fi, object_normal * fn)
{
    return new_object(gs().buf, fi, fn);
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
    const char * const out_path = (argc == 2) ? NULL : argv[2];
    observer o = {
        .eye = {gr(), gr(), gr()},
        .view = {gr(), gr(), gr()},
        .column_direction = {gr(), gr(), gr()},
        .row_direction = {gr(), gr(), gr()},
        .width = atoi(dim_w), .height = atoi(dim_h)
    };
    int c = gi();
    if (c <= 0) fail("no scene objects\n");
    world world_ = { color_sky, NULL, 0, { c, new scene_object[c] } };
    sky_color = {1, 1, 1};
    void ** args = new void *[c];
    void ** decoration_args = new void *[c];
    int j = 0;
    for (int i = 0; i < world_.scene_.object_count; i++) {
        object_intersection fi;
        object_normal fn;
        bufstr_256 bs = gs();
        char * class_name = bs.buf;
        void * a = (strcmp(class_name, "x"))
            ? new_object(class_name, &fi, &fn)
            : new_inter(&fi, &fn, gi(), new_member);
        if (!a) fail("object [%d] error\n", i);
        args[i] = a;

        object_decoration df = NULL;
        void * d = NULL;
        const char * buf = gs().buf;
        real gr_;
        if (isalpha(buf[0])) {
            d = new_decoration(buf, &df);
            if ( ! d) fail("decoration [%d] error\n", i);
            decoration_args[j++] = d;
            gr_ = gr();
        } else {
            const int n = sscanf(buf, REAL_FMT, &gr_);
            if (n != 1) fail("optics [%d] error\n", i);
        }
        scene_object o = { fi, fn, a,
            {
                {gr_, gr(), gr()},
                {gr(), gr(), gr()},
                gr(),
                {gr(), gr(), gr()},
                {gr(), gr(), gr()}
            },
            df, d
        };
        world_.scene_.objects[i] = o;
    }

    bufstr_256 bs = gs();
    char * sky_name = bs.buf;
    if (strcmp(sky_name, "rgb") == 0) {
        world_.sky = rgb_sky;
    } else if (strcmp(sky_name, "color") == 0) {
        color sky = { gr(), gr(), gr() };
        sky_color = sky;
        world_.sky = color_sky;
    } else {
        sky_photo = photo_create(sky_name);
        if ( ! sky_photo) fail("could not load sky '%s'\n", sky_name);
        world_.sky = photo_sky;
    }

    const int k = gi();
    world_.spot_count = k;
    world_.spots = new light_spot[k];
    for (int x=0; x<k; x++) {
        light_spot s = {
            {gr(), gr(), gr()},
            {gr(), gr(), gr()}
        };
        world_.spots[x] = s;
    }
    while (fgetc(stdin) != EOF)
        ; // wait till we get end-of-file (polite to not break the pipe)

    image out = image_create(out_path, o.width, o.height);
    if (report_status)
        fprintf(stderr, "Tracing %dx%d of Observer\ny", o.width, o.height);
    for (int row = 0; row < o.height; row++) {
        for (int column = 0; column < o.width; column++) {
            ray ray_ = observer_ray(&o, column, row);
            image_write(out, trace(ray_, &world_));
        }
        if (report_status) fprintf(stderr, "\r%d", row);
    }
    image_close(out);

    for (int q=0; q<c; q++) delete_object_or_inter(args[q]);
    while (--j>=0) delete_texture_mapping(decoration_args[j]);
    delete [] args;
    delete [] decoration_args;
    delete [] world_.scene_.objects;
    delete [] world_.spots;
    if (report_status) fprintf(stderr, "\n");
}
