//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef INTER_H
#define INTER_H

#include "scene.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void * (*object_generator)
    (object_intersection * fi, object_normal * fn, void * get_state);
void * make_inter(object_intersection * fi, object_normal * fn,
        int m, object_generator get, void * get_state);
void delete_inter(void * a);

#ifdef __cplusplus
}
#endif

#endif
