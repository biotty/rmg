//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef UNITFWD_HPP
#define UNITFWD_HPP

#include <memory>

struct generator;
typedef std::shared_ptr<generator> ug_ptr;
#define P std::make_shared

// consider: misplaced
#define SR 44100
#define SC 21

#endif

