//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef MATRIX_H
#define MATRIX_H

#include "real.h"

#ifdef __cplusplus
extern "C" {
#endif

void multiply(const real * matrix, int columns, int rows, const real * m, real * r);

#ifdef __cplusplus
}
#endif

#endif

