
#include "bitarray.h"

#include <stdlib.h>
#include <assert.h>


    static inline int
array_length(int bit_count)
{
    return 1 + (bit_count - 1) / 8;
}

    int
ba_size(int bit_count)
{
    return (sizeof (bitarray)) + array_length(bit_count);
}

    static inline div_t
position(bitarray * ba, int i)
{
    assert(i >= 0);
    assert(i < (int)ba->bit_count);
    return div(i, 8);
}

    void
ba_set(bitarray * ba, int i)
{
    const div_t p = position(ba, i);
    ba->c[p.quot] |= 1 << p.rem;
}

    void
ba_clear(bitarray * ba, int i)
{
    const div_t p = position(ba, i);
    ba->c[p.quot] &= ~ (1 << p.rem);
}

    void
ba_toggle(bitarray * ba, int i)
{
    const div_t p = position(ba, i);
    ba->c[p.quot] ^= 1 << p.rem;
}

    void
ba_assign(bitarray * ba, int i, bool b)
{
    if (b) ba_set(ba, i);
    else ba_clear(ba, i);
}

    bool
ba_isset(bitarray * ba, int i)
{
    const div_t p = position(ba, i);
    return ba->c[p.quot] & (1 << p.rem);
}

    static inline int
byte_firstset(register int b)
{
    register int i = 0;
    while ((b & 1) == 0) {
        b >>= 1;
        if (++i > 7)
            return -1;
    }
    return i;
}

    int
ba_firstset(bitarray * ba)
{
    for (int x=0; x<array_length(ba->bit_count); x++)
        if (ba->c[x])
            return x * 8 + byte_firstset(ba->c[x]);
    return -1;
}

    static inline int
byte_lastset(register int b)
{
    register int i = 7;
    while ((b & 128) == 0) {
        b <<= 1;
        if (--i < 0)
            return -1;
    }
    return i;
}

    int
ba_lastset(bitarray * ba)
{
    for (int x=array_length(ba->bit_count)-1; x>=0; x--)
        if (ba->c[x])
            return x * 8 + byte_lastset(ba->c[x]);
    return -1;
}

