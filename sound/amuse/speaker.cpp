//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "speaker.hpp"

#include "unit.hpp"
#include "SDL.h"

void speaker::put(double y)
{
    if (y > 1) y = 1;
    else if (y < -1) y = -1;
    b.put(y * 32767);
}

speaker::speaker() : g(), b(4096)
{
    init_sdl_audio(this);
}

speaker::~speaker() { SDL_CloseAudio(); }

bool speaker::produce()
{
    unsigned i = 0;
    if (g->more()) {
        while (b.count() + SU <= b.n) {
            ++i;
            unit u;
            g->generate(u);
            SDL_LockAudio();
            FOR_SU(i) put(u.y[i]);
            SDL_UnlockAudio();
        }
    }
    return i;
}

void speaker_callback(void *u, Uint8 *s, int n)
{
    for (int i=0; i<n; i+=2) ((speaker *)u)->b.get().set(&s[i]);
}

void init_sdl_audio(speaker * sp)
{
    static bool i = false;
    if (i) fprintf(stderr, "SDL already inited\n");
    SDL_AudioSpec s;
    s.freq = SR;
    s.format = AUDIO_S16;
    s.channels = 1;
    s.samples = 1024;
    s.callback = speaker_callback;
    s.userdata = (void *)sp;
    if (SDL_OpenAudio(&s, NULL) < 0)
        fprintf(stderr, "%s\n", SDL_GetError());
    i = true;
}

