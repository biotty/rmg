//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef OBSERVER_H
#define OBSERVER_H

#include "ray.h"

struct observer {
	point eye;
	point view;
	direction column_direction;
	direction row_direction;
};

#ifndef __cplusplus
typedef struct observer observer;
#endif

#ifdef __cplusplus
extern "C" {
#endif

ray observer_ray(const observer *, real aspect_ratio,
        real unit_column, real unit_row);

#ifdef __cplusplus
}
#endif

#endif

