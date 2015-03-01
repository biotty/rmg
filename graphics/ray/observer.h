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
	int width, height;
};

#ifndef __cplusplus
typedef struct observer observer;
#endif

#ifdef __cplusplus
extern "C" {
#endif

ray observer_ray(const observer *, int column, int row);

#ifdef __cplusplus
}
#endif

static inline real observer_x(const observer * o, int column)
{
	return column /(real) o->width;
}

static inline real observer_y(const observer * o, int row)
{
	return row /(real) o->height;
}

#endif

