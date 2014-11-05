//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "bitarray.h"

#include "math.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

    int
main(void)
{
    const int count = 30;
    bitarray * ba = calloc(1, ba_size(count));
    ba->bit_count = count;
    assert(ba_firstset(ba) < 0);
    assert(ba_lastset(ba) < 0);
    ba_toggle(ba, 23);
    assert(ba_isset(ba, 23));
    ba_toggle(ba, 23);
    assert( ! ba_isset(ba, 23));
    ba_assign(ba, 0, true);
    ba_assign(ba, 23, true);
    assert(ba_isset(ba, 23));
    assert(ba_firstset(ba) == 0);
    ba_clear(ba, 0);
    //printf("%d\n", ba_firstset(ba));
    assert(ba_firstset(ba) == 23);
    assert(ba_lastset(ba) == 23);
}
