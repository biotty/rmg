//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "bitarray.hpp"
#include <algorithm>


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
    auto it = std::find(bits.begin(), bits.end(), true);
    if (bits.end() == it) return -1;

    return std::distance(bits.begin(), it);
}
