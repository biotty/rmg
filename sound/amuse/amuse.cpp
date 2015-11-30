//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "speaker.hpp"
#include "screen.hpp"
#include "unit.hpp"
#include <algorithm>

int main()
{
    screen scr;
    speaker spk(SR);
    generator * g = nullptr;
    while (scr.editing()) {
        if ( ! scr.running() && scr.eager()) {
            g = scr.mastered();
            SDL_PauseAudio(0);
            scr.start();
        }
        if (scr.running() && ! scr.eager()) {
            SDL_Delay(1 + spk.samples() * 1000 / SR);
            SDL_PauseAudio(1);
            scr.stop();
        }
        if (scr.running() && g->more()) {
            while (spk.asked() > unit::size) {
                unit u;
                g->generate(u);
                SDL_LockAudio();
                std::for_each(std::begin(u.y), std::end(u.y),
                        [&spk](double q){ spk.put(q); });
                SDL_UnlockAudio();
            }
        } else {
            SDL_Delay(10);
        }

        scr.edit();
    }
}

