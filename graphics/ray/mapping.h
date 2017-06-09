//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef MAPPING_H
#define MAPPING_H

#include "scene.h"

struct texture_application {
    real x_wrap;
    real y_wrap;
    color reflection_factor;
    color absorption_factor;
    color refraction_factor;
};

#ifndef __cplusplus
typedef struct texture_application texture_application;
#endif

#ifdef __cplusplus
extern "C" {
#endif

void * normal_texture_mapping(
        object_decoration * df, direction n,
        const char * path, texture_application a);

void * planar_texture_mapping(
        object_decoration * df, direction n,
        const char * path, texture_application a);

void * relative_texture_mapping(
        object_decoration * df, direction n, point o,
        const char * path, texture_application a);

void * axial_texture_mapping(
        object_decoration * df, direction n, point o,
        const char * path, texture_application a);

void delete_texture_mapping(void * decoration_arg);

#ifdef __cplusplus
}
#endif

#endif
