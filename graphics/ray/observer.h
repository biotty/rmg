//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef OBSERVER_H
#define OBSERVER_H

#include "ray.h"

typedef struct {
	point eye;
	point view;
	direction column_direction;
	direction row_direction;
	int width, height;
} observer;

ray observer_ray(const observer *, int column, int row);

static inline real observer_x(const observer * o, int column)
{
	return column /(real) o->width;
}

static inline real observer_y(const observer * o, int row)
{
	return row /(real) o->height;
}

#endif

