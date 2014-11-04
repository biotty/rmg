#ifndef SPEAKER_HPP
#define SPEAKER_HPP

#include "ringbuf.hpp"
#include "unitfwd.hpp"
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
    void put(double y);
public:
    ug_ptr g;
    ringbuf<e> b;
    speaker(unsigned sr);
    ~speaker();
    bool produce();
};

void speaker_callback(void *u, Uint8 *stream, int len);
void init_sdl_audio(unsigned sr, speaker * sp);

#endif

