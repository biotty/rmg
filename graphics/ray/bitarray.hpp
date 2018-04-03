//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef BITARRAY_H
#define BITARRAY_H

#include <vector>

class bitarray {
    std::vector<bool> bits;
public:
    bitarray(int q);
    void flip(int i);
    void assign(int i, bool);
    bool isset(int i) const;
    int firstset() const;
};


#endif
