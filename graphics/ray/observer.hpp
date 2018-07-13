//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef OBSERVER_HPP
#define OBSERVER_HPP

#include "observer.h"
#include "trace.hpp"

    void
produce_trace(const char * path, int width, int height,
        const world & w, const observer & o, unsigned n_threads);


#endif
