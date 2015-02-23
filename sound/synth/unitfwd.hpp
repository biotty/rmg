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
#define U make_unique

// patch: c++11 doesn't yet have
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// consider: misplaced
#define SR 44100
#define SC 21

#endif

