//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef CREATORDECL_HPP
#define CREATORDECL_HPP

#include "unitfwd.hpp"

struct builder
{
    virtual ug_ptr build() = 0;
};

typedef std::shared_ptr<builder> bu_ptr;

#endif

