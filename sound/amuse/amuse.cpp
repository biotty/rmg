//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "speaker.hpp"
#include "screen.hpp"

int main()
{
    screen scr;
    speaker spk;
    while (scr.editing()) {
        if ( ! scr.running() && scr.eager()) {
            spk.g = scr.mastered();
            SDL_PauseAudio(0);
            scr.start();
        }
        if (scr.running() && ! scr.eager()) {
            SDL_Delay(1 + spk.b.n * 1000 / SR);
            SDL_PauseAudio(1);
            scr.stop();
        }
        if ( ! (scr.running() && spk.produce()))
            SDL_Delay(10);

        scr.edit();
    }
}

