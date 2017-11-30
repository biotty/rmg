//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef UNITFWD_HPP
#define UNITFWD_HPP

#include <memory>

struct unit;
struct generator
{
    virtual void generate(unit & u) = 0;
    virtual bool more() = 0;
    virtual ~generator();
};

typedef std::unique_ptr<generator> ug_ptr;
#define P std::make_shared
#define U std::make_unique

#define SR 44100

struct builder
{
    virtual ug_ptr build() = 0;
    virtual ~builder();
};

typedef std::unique_ptr<builder> bu_ptr;
typedef std::shared_ptr<builder> bs_ptr;

#endif
