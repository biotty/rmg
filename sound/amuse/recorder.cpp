//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "recorder.hpp"

#include "unit.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdint>

namespace {

void put(double v, FILE * f)
{
    if (v > 1) v = 1;
    else if (v < -1) v = -1;
    int16_t b = v * 32767;
    fwrite(&b, 2, 1, f);
}

}

recorder::recorder(const char * path) : out(fopen(path, "wb")) {}

void recorder::run(generator & g)
{
    while (g.more()) {
        unit u;
        g.generate(u);
        FILE * o = out;
        std::for_each(std::begin(u.y), std::end(u.y),
                [o](double q){ ::put(q, o); });
    }
}

recorder::~recorder() { fclose(out); }

