//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef XYZ_H
#define XYZ_H

#include "real.h"

struct xyz {
	real x;
	real y;
	real z;
};

#ifndef __cplusplus
typedef struct xyz xyz;
#endif

#endif

