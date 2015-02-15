//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "recorder.hpp"

#include "unit.hpp"
#include <cstdio>
#include <cstdint>

void put(double v, FILE * f)
{
    if (v > 1) v = 1;
    else if (v < -1) v = -1;
    int16_t b = v * 32767;
    fwrite(&b, 2, 1, f);
}

recorder::recorder(const char * path) : out(fopen(path, "wb")) {}

void recorder::run(ug_ptr g)
{
    while (g->more()) {
        unit u;
        g->generate(u);
        FOR_SU(i) put(u.y[i], out);
    }
}

recorder::~recorder() { fclose(out); }

