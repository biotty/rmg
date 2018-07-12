//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef OBSERVER_H
#define OBSERVER_H

#include "trace.hpp"

struct observer {
    point eye;
    point view;
    direction column_direction;
    direction row_direction;
};


    void
produce_trace(const char * path, int width, int height,
        const world & w, const observer & o, unsigned n_threads);


#endif
