//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "speaker.hpp"

void speaker_callback(void *u, Uint8 *stream, int len);
void init_sdl_audio(speaker * sp, unsigned sr);

speaker::speaker(unsigned sr) : b(4096)
{
    init_sdl_audio(this, sr);
}

unsigned speaker::asked()
{
    return b.n - b.count();
}

void speaker::put(double y)
{
    if (y > 1) y = 1;
    else if (y < -1) y = -1;
    b.put(y * 32767);
}

unsigned speaker::samples()
{
    return b.count();
}

void speaker::readsample(unsigned char * a)
{
    b.get().set(a);
}

void speaker_callback(void *u, Uint8 *s, int n)
{
    for (int i=0; i<n; i+=2) ((speaker *)u)->readsample(&s[i]);
}

speaker::~speaker() { SDL_CloseAudio(); }

void init_sdl_audio(speaker * sp, unsigned sr)
{
    static bool i = false;
    if (i) fprintf(stderr, "SDL already inited\n");
    SDL_AudioSpec s;
    s.freq = sr;
    s.format = AUDIO_S16;
    s.channels = 1;
    s.samples = 1024;
    s.callback = speaker_callback;
    s.userdata = (void *)sp;
    if (SDL_OpenAudio(&s, NULL) < 0)
        fprintf(stderr, "%s\n", SDL_GetError());
    i = true;
}

