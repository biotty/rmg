#ifndef RECORDER_HPP
#define RECORDER_HPP

#include "unitfwd.hpp"

struct generator;
typedef std::shared_ptr<generator> ug_ptr;

struct recorder
{
    FILE * out;
    recorder(const char * path);
    void run(ug_ptr g);
    ~recorder();
};

#endif

