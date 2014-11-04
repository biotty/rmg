#include "book.hpp"
#include "builders.hpp"
#include "musicm.hpp"

#include <ncurses.h>

#define FOR(X, A, B) for (unsigned X=A; X!=B; ++X)

book::book(screen * s) : copy_from(), s(*s)
{
    FOR(i, 0, 100) pages.push_back(P<page>(s, i));
    flip(0);
}

void book::unmark()
{
    FOR(i, 0, pages.size()) pages[i]->m = 0;
}

unsigned book::effect_end(unsigned k, unsigned p, unsigned q)
{
    if ( ! s.o->is_effect(pages[p]->a[k].i)) throw nullptr;
    FOR(i, p+1, q) {
        if ( ! s.o->is_effect(pages[p]->a[k].i)
                || pages[i]->a[k].i != pages[p]->a[k].i
                || pages[i]->a[k].u != pages[p]->a[k].u
                || pages[i]->a[k].l != pages[p]->a[k].l)
            return i;
    }
    return q;
}

mv_ptr book::glide(unsigned k, unsigned p, unsigned q)
{
    unsigned h = pages[p]->pitch(k, 0);
    if ( ! h) h = 72;
    unsigned b = pages[q - 1]->pitch(k, BE - 1);
    if ( ! b) b = h;

    double t = MAX_T_AHEAD;
    double z = t + (q - p) * s.duration();
    double d = s.duration() / BE;
    pe_ptr e = P<punctual>(f_of_just(h), f_of_just(b));
    FOR(i, p, q) {
        FOR(j, 0, BE) {
            if (i || j)
                if (unsigned y = pages[i]->pitch(k, j))
                    e->p(t / z, f_of_just(y));
            t += d;
        }
    }
    return P<movement>(e, z);
}

bu_ptr book::link(unsigned k, unsigned p, unsigned q)
{
    if ( ! k) return bu_ptr();

    sc_ptr sc = P<score>();
    while (k) {
        --k;
        FOR(i, p, q) {
            if (pages[i]->m & (1 << k)) continue;
            pages[i]->m |= (1 << k);

            double t = (i - p) * s.duration();

            if (s.o->is_effect(pages[i]->a[k].i)) {
                unsigned e = effect_end(k, i, q);
                FOR(j, i, e) pages[j]->m |= (1 << k);
                sc->add(t, s.o->effect(pages[i]->a[k].i,
                            link(k, i, e),
                            glide(k, i, e),
                            pages[i]->a[k].u * s.duration() / BE,
                            1 - pages[i]->a[k].l/99.0)
                       );
            } else {
                double x = t + MAX_T_AHEAD;
                for (unsigned j=0; j<BE; j++, x += s.duration() / BE)
                    if (sound_entry y = pages[i]->play(k, j))
                        sc->add(x - y.a, y.s);
            }
        }
    }
    return sc;
}

void book::print_h()
{
    mvprintw(s.nr(), 0, "%02d", current->h);
    current->draw();
}

void book::flip(unsigned i)
{
    current = pages.at(i).get();
    print_h();
}

void book::handle(int ch)
{
    if (ch == KEY_NPAGE) flip((current->h + 1) % 100);
    else if (ch == KEY_PPAGE) flip((current->h + 99) % 100);
    else if (ch == '@') copy_from = current;
    else if (ch == '!' && copy_from) {
        unsigned h = current->h;
        *current = *copy_from;
        current->h = h;
        flip(h);
    }
    else if (ch == 'p') s.speak(s.compile(0, current->h + 1));
    else if (ch == 'P') s.speak(s.compile(current->h, current->h + 1));
    else current->handle(ch);
}

