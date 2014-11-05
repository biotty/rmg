//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "screen.hpp"
#include "page.hpp"
#include "book.hpp"

#include "recorder.hpp"
#include "builders.hpp"
#include "generators.hpp"
#include "musicm.hpp"

#include <ncurses.h>

void init_ncurses();

void screen::print_m()
{
    mvprintw(nr(), 2, "%02d", m);
    mvchgat(nr(), 2, 2, A_REVERSE, 0, NULL);
}

void screen::write_m(unsigned i)
{
    m = i;
    print_m();
}

double screen::duration() { return 60.0 / m; }

void screen::print_r()
{
    mvprintw(nr(), 4, "%s", r ? "r" : "s");
}

void screen::write_r(bool i)
{
    r = i;
    print_r();
}

screen::screen(unsigned sr)
    : z(), r(), g(P<sum>()), n(), m(30)
    , k(new book(this)), o(new orchestra())
{
    init_ncurses();
    k->print_h();
    print_m();
    ug_global.sr = sr;
    fl_global.sr = sr;
}

screen::~screen()
{
    delete o;
    delete k;
    endwin();
}

sum * screen::g_ptr() { return dynamic_cast<sum *>(g.get()); }

void screen::speak(ug_ptr c)
{
    sum * z = g_ptr();
    z->c(c, 8 / (9 + double(z->s.size())));
}

void screen::shut() { g_ptr()->s.resize(0); }

bool screen::editing() { return !z; }
bool screen::running() { return r; }
void screen::start() { if (!r) write_r(true); }
void screen::stop() { if (r) write_r(false); }

void screen::edit()
{
    const int ch = getch();
    if (ch == ERR) return;
    else if (ch == 'k') k->flip(consume_n());
    else if (ch == 'm') {
        if (unsigned i = consume_n()) write_m(i);
    }
    else if (ch >= '0' && ch <= '9') {
        n = 10 * n + (ch - '0');
        return;
    }
    else if (ch == ';') shut();
    else k->handle(ch);
    consume_n();
    if (ch == 'Z') z = true;
}

bool screen::eager() { return g->more(); }

ug_ptr screen::mastered()
{
    return P<limiter>(g);
}

unsigned screen::consume_n()
{
    const unsigned r = n;
    n = 0;
    return r;
}

void init_ncurses()
{
    static bool i = false;
    if (i) {
        fprintf(stderr, "screen already initiated\n");
        return;
    }
    i = true;
    initscr();
    curs_set(0);
    noecho();
    cbreak();
    nodelay(stdscr, true);
    keypad(stdscr, true);
    notimeout(stdscr, true);
}

unsigned screen::nr() { return 10; } //must be less than bits in unsigned page::m

void screen::store(ug_ptr g)
{
    recorder r("r.wav");
    r.run(g);
}

ug_ptr screen::compile(unsigned p, unsigned q)
{
    k->unmark();
    return k->link(nr(), p, q)->build();
}
