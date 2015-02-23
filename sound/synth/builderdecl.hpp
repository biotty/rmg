//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef BUILDERDECL_HPP
#define BUILDERDECL_HPP

#include "unitfwd.hpp"

struct builder
{
    virtual ug_ptr build() = 0;
    virtual ~builder();
};

typedef std::shared_ptr<builder> bu_ptr;

#endif

