//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef STACK_H
#define STACK_H

#include <vector>
#include <cstdint>

class stack {
    std::vector<int16_t> values;
public:
    stack();
    void push(int16_t v);
    int16_t pop();
};

#endif
