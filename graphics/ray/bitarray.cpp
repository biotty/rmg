//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "bitarray.hpp"


bitarray::bitarray(int q) : bits(q) {}

    void
bitarray::flip(int i) { bits[i].flip(); }

    void
bitarray::assign(int i, bool b) { bits[i] = b; }

    bool
bitarray::isset(int i) const { return bits[i]; }

    int
bitarray::firstset() const
{
    const unsigned n = bits.size();
    for (unsigned i=0; i < n; i++) {
        if (bits[i]) return i;
    }
    return -1;
}
