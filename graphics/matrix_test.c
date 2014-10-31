
#include "matrix.h"

#include "math.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

    int
main(void)
{
    real matrix[6] = {1, 2, 3, 4, 5, 6};
    real r[2], m[3] = {7, 8, 9};
    multiply(matrix, 3, 2, m, r);
    //printf(REAL_FMT "\n", r[0]);
    assert(is_pretty_near(r[0], 50));
    assert(is_pretty_near(r[1], 122));
}

