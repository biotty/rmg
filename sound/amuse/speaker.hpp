//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef SPEAKER_HPP
#define SPEAKER_HPP

#include "ringbuf.hpp"
#include "SDL.h"

class speaker
{
    struct e {
        int16_t w;

        e(int i = 0) : w(i) {}
        void set(unsigned char * p)
        {
            char * b = (char *)&w;
            p[0] = b[0];
            p[1] = b[1];
        }
    };
    ringbuf<e> b;
    void readsample(unsigned char * a);
    friend void speaker_callback(void *u, Uint8 *s, int n);
public:
    unsigned asked();
    void put(double y);
    unsigned samples();
    speaker(unsigned sr);
    ~speaker();
};

#endif

