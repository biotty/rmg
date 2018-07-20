//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef RENDER_HPP
#define RENDER_HPP

#include "trace.hpp"
#include "observer.h"

    void
render(const char * path, int width, int height,
        const observer & obs, const world & w, unsigned n_threads);


#endif
