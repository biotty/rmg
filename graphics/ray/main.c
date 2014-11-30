//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "map.h"
#include "inter.h"
#include "observer.h"
#include "trace.h"
#include "image.h"
#include "xmath.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

char * strdup(const char * s);

typedef struct {
    char buf[256];
} string;

    static void
fail(const char * fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}

#define SCAN_DEFINITION(T, N, F, R) static T g ## N (void) \
{ T N; if (1 != scanf(F, R)) fail("%s\n", F); return N; }
SCAN_DEFINITION(char, c, "%c", &c)
SCAN_DEFINITION(int, i, "%d", &i)
SCAN_DEFINITION(real, r, REAL_FMT, &r)
SCAN_DEFINITION(string, s, "%s", s.buf)

    void *
new_object(const char * object_class,
        object_intersection * fi, object_normal * fn)
{
    object_arg_union * object_arg = NULL;
    if (0 == strcmp(object_class, "plane")) {
        plane plane_ = {
            .point = {gr(), gr(), gr()},
            .normal = {gr(), gr(), gr()}};
        normalize(&plane_.normal);
        object_arg = malloc(sizeof *object_arg);
        object_arg->plane = plane_;
        *fi = plane_intersection;
        *fn = plane_normal;
    } else if (0 == strcmp(object_class, "sphere")
            || 0 == strcmp(object_class, "minusphere")) {
        sphere sphere_ = {
            .center = {gr(), gr(), gr()},
            .radius = gr()};
        object_arg = malloc(sizeof *object_arg);
        object_arg->sphere = sphere_;
        if (memcmp(object_class, "minu", 4) != 0) {
            *fi = sphere_intersection;
            *fn = sphere_normal;
        } else {
            *fi = minusphere_intersection;
            *fn = minusphere_normal;
        }
    } else if (0 == strcmp(object_class, "cylinder")
            || 0 == strcmp(object_class, "minucylinder")) {
        real r, theta, phi;
        direction p = {gr(), gr(), gr()};
        direction axis = {gr(), gr(), gr()};
        spherical(axis, &r, &theta, &phi);
        cylinder cylinder_ = {{-p.x, -p.y, -p.z}, square(r), theta, phi};
        object_arg = malloc(sizeof *object_arg);
        object_arg->cylinder = cylinder_;
        if (memcmp(object_class, "minu", 4) != 0) {
            *fi = cylinder_intersection;
            *fn = cylinder_normal;
        } else {
            *fi = minucylinder_intersection;
            *fn = minucylinder_normal;
        }
    } else if (0 == strcmp(object_class, "cone")
            || 0 == strcmp(object_class, "minucone")) {
        real r, theta, phi;
        direction apex = {gr(), gr(), gr()};
        direction axis = {gr(), gr(), gr()};
        spherical(axis, &r, &theta, &phi);
        cone cone_ = {{-apex.x, -apex.y, -apex.z}, 1/r, theta, phi};
        object_arg = malloc(sizeof *object_arg);
        object_arg->cone = cone_;
        if (memcmp(object_class, "minu", 4) != 0) {
            *fi = cone_intersection;
            *fn = cone_normal;
        } else {
            *fi = minucone_intersection;
            *fn = minucone_normal;
        }
    } else {
        fail("object class \"%s\"?\n", object_class);
    }
    return object_arg;
}

    map_application
get_map_application(void)
{
    return (map_application){
        .x_wrap = gr(),
        .y_wrap = gr(),
        .reflection_factor = {gr(), gr(), gr()},
        .reflection_adjust = {gr(), gr(), gr()},
        .absorption_factor = {gr(), gr(), gr()},
        .absorption_adjust = {gr(), gr(), gr()},
        .refraction_index = gr(),
        .refraction_factor = {gr(), gr(), gr()},
        .refraction_adjust = {gr(), gr(), gr()},
        .traversion_factor = {gr(), gr(), gr()},
        .traversion_adjust = {gr(), gr(), gr()},
    };
}

    void *
new_decoration(const char * deco_name,
        object_decoration * df)
{
    if (strcmp(deco_name, "map") == 0) {
        return map_decoration(df, & (n_map_setup){
            .n = {gr(), gr(), gr()},
            .path = gs().buf,
            .a = get_map_application()});
    } else if (strcmp(deco_name, "pmap") == 0) {
        return pmap_decoration(df, & (n_map_setup){
            .n = {gr(), gr(), gr()},
            .path = gs().buf,
            .a = get_map_application()});
    } else if (strcmp(deco_name, "omap") == 0) {
        return omap_decoration(df, & (n_o_map_setup){
            .n = {gr(), gr(), gr()},
            .o = {gr(), gr(), gr()},
            .path = gs().buf,
            .a = get_map_application()});
    } else if (strcmp(deco_name, "lmap") == 0) {
        return lmap_decoration(df, & (n_o_map_setup){
            .n = {gr(), gr(), gr()},
            .o = {gr(), gr(), gr()},
            .path = gs().buf,
            .a = get_map_application()});
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
    const bool report_status = getenv("CRAY_RS") != NULL;
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
        .width = atoi(dim_w), .height = atoi(dim_h)};
    const int n = gi();
    if (gc() != ':') fail("object-count colon missing\n");
    world world_ = alloc_world(n);
    void * args[n];
    void * decoration_args[n];
    int j = 0;
    for (int i = 0; i < n; i++) {
        object_intersection fi;
        object_normal fn;
        char * class_name = strdup(gs().buf);
        void * a = (strcmp(class_name, "inter"))
            ? new_object(class_name, &fi, &fn)
            : new_inter(&fi, &fn, gi(), new_member);
        free(class_name);
        if (!a) fail("object [%d] error\n", i);
        args[i] = a;

        const char * buf = gs().buf;
        if (isalpha(buf[0])) {
            object_decoration df;
            void * d = new_decoration(buf, &df);
            if ( ! d) fail("decoration [%d] error\n", i);
            decoration_args[j] = d;
            ++j;
            set_object(world_, i, (scene_object){ fi, fn, a,
                    .decoration = df,
                    .decoration_arg = d,
                    });
        } else {
            real gr_;
            const int n = sscanf(buf, REAL_FMT, &gr_);
            if (n != 1) fail("optics [%d] error\n", i);
            set_object(world_, i, (scene_object){ fi, fn, a,
                    .reflection_filter = {gr_, gr(), gr()},
                    .absorption_filter = {gr(), gr(), gr()},
                    .refraction_index = gr(),
                    .refraction_filter = {gr(), gr(), gr()},
                    .traversion_filter = {gr(), gr(), gr()},
                    });
        }
    }
    char * sky_name = strdup(gs().buf);
    if (strcmp(sky_name, "funky") == 0) set_sky(world_, funky_sky);
    if (strcmp(sky_name, "photo") == 0) {
        sky_photo = photo_create("sky.pnm");
        if ( ! sky_photo) {
            fprintf(stderr, "trying sky.jpeg\n");
            sky_photo = photo_create("sky.jpeg");
        }
        if ( ! sky_photo) fail("could not load sky photo\n");
        set_sky(world_, photo_sky);
    }
    free(sky_name);
    const int k = gi();
    if (gc() != ':') fail("light-spot-count colon missing\n");
    light_spot spots[k];
    for (int x=0; x<k; x++) {
        spots[x] = (light_spot){
            .point = {gr(), gr(), gr()},
            .color = {gr(), gr(), gr()}
        };
    }
    set_spots(world_, spots, k);
    image out = image_create(out_path, o.width, o.height);
    if (report_status) fprintf(stderr, "Tracing view of Observer\n");
    for (int row = 0; row < o.height; row++) {
        for (int column = 0; column < o.width; column++) {
            ray ray_ = observer_ray(&o, column, row);
            image_write(out, trace(ray_, world_));
        }
        if (report_status) fprintf(stderr, "\r%d", row);
    }
    for (int q=0; q<n; q++) free(args[q]);
    while (--j>=0) generic_map_delete(decoration_args[j]);
    destroy_world(world_);
    if (report_status) fprintf(stderr, "\n");
}

