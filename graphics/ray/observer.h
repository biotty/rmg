//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef OBSERVER_H
#define OBSERVER_H

#include "point.h"
#include "direction.h"

struct observer {
    point eye;
    point view;
    direction column_direction;
    direction row_direction;
};

#ifndef __cplusplus
typedef struct observer observer;
#else

extern "C" {
#endif

void direct_row(observer * o);

#ifdef __cplusplus
}
#endif

#endif
