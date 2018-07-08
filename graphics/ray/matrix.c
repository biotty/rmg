//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "matrix.h"

void multiply(const real * restrict matrix, int columns, int rows,
        const real * restrict m, real * restrict r)
{
    for (int row=0; row<rows; row++) {
        const real * matrix_row = & matrix[row * columns];
        real sum_ = 0;
        for (int column=0; column<columns; column++)
            sum_ += matrix_row[column] * m[column];
        r[row] = sum_;
    }
}

