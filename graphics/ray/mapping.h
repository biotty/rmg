//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef MAPPING_H
#define MAPPING_H

#include "scene.h"

struct texture_application {
    color reflection_factor;
    color absorption_factor;
    color refraction_factor;
};

#ifndef __cplusplus
typedef struct texture_application texture_application;
#else
extern "C" {
#endif

void * normal_texture_mapping(
        object_decoration * df, tilt_arg rota, real r,
        real w, const char * path, texture_application a);

void * planar_texture_mapping(
        object_decoration * df, tilt_arg rota, real r,
        real w, point o, const char * path, texture_application a);

void * planar1_texture_mapping(
        object_decoration * df, tilt_arg rota, real r,
        real w, point o, const char * path, texture_application a);

void * relative_texture_mapping(
        object_decoration * df, tilt_arg rota, real r,
        real w, point o, const char * path, texture_application a);

void * axial_texture_mapping(
        object_decoration * df, tilt_arg rota, real r,
        real w, point o, const char * path, texture_application a);

void * axial1_texture_mapping(
        object_decoration * df, tilt_arg rota, real r,
        real w, point o, const char * path, texture_application a);

void * checkers_mapping(
        object_decoration * df, tilt_arg rota, real r,
        real w, point o, int q,
        compact_color reflection_filter,
        compact_color absorption_filter,
        compact_color refraction_filter);

void delete_decoration(void * decoration_arg);

#ifdef __cplusplus
}
#endif

#endif
