//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "stack.hpp"


stack::stack() : values() {}

    void
stack::push(int16_t v)
{
    values.push_back(v);
}

    int16_t
stack::pop()
{
    if (values.empty()) return -1;

    int16_t v = values.back();
    values.pop_back();
    return v;
}
