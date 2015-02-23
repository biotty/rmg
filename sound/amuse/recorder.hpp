//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef RECORDER_HPP
#define RECORDER_HPP

#include "unitfwd.hpp"

struct recorder
{
    FILE * out;
    recorder(const char * path);
    void run(generator & g);
    ~recorder();
};

#endif

