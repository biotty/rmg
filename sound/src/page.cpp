#include "page.hpp"
#include "screen.hpp"
#include "builders.hpp"
#include "recorder.hpp"
#include "musicm.hpp"

#include <cmath>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <ncurses.h>

namespace {

const char * scalenotes = "CdDeEFgGaAbB";

unsigned scalekey(char n)
{
    if (!isalpha(n)) return 0;
    n ^= 32;
    const char * p = std::strchr(scalenotes, n);
    return p ? 12 + (p - scalenotes) : 0;
}

const char * pitchname(unsigned pitch)
{
    if (pitch < 12) return "  ";
    static char n[3] = {};
    n[0] = scalenotes[pitch % 12];
    n[1] = '0' + pitch / 12 - 1;
    return n;
}

} //namespace

page::ruler::cell::cell() : p() {}
page::ruler::ruler() : i(), l(), u(8) {}
page::page(screen * s, unsigned h)
    : s(s), c(), t(), h(h), o(4), m(), a(s->nr())
{}

page::ruler::cell * page::cell(unsigned k, unsigned i)
{
    auto p = a[k].e.find(i);
    if (p == a[k].e.end())
        return nullptr;
    else
        return & p->second;
}

unsigned page::pitch(unsigned k, unsigned i)
{
    if (ruler::cell * e = cell(k, i))
        return e->p;
    else
        return 0;
}

sound_entry page::play(unsigned k, unsigned i)
{
    if (ruler::cell * e = cell(k, i)) {
        return s->o->play(a[k].i,
                instruction(f_of_just(e->p), a[k].u * s->duration() / BE,
                    a_of(-double(a[k].l)), e->q));
    } else {
        return sound_entry();
    }
}

int page::x_of(unsigned i) { return 6 + 2*i; }

void page::print_i(int k)
{
    mvprintw(k, 0, "%02d", a[k].i);
    mvchgat(k, 0, 2, A_REVERSE, 0, NULL);
}
void page::print_u(int k) { mvprintw(k, 2, "%02d", a[k].u); }
void page::print_l(int k)
{
    mvprintw(k, 4, "%02d", a[k].l);
    mvchgat(k, 4, 2, A_REVERSE, 0, NULL);
}
void page::print_w()
{
    if (ruler::cell * e = cell(c, t))
        for (unsigned i=0; i<8; i++)
            mvprintw(a.size(), 6+i*4, "%2d%d#", e->q.a[i], 1 + i);
    else mvprintw(a.size(), 6, " 01# 02# 03# 04# 05# 06# 07# 08#");
}

void page::draw()
{
    for (unsigned k=0; k<a.size(); k++) {
        print_i(k);
        print_u(k);
        print_l(k);
        for (unsigned i=0; i<BE; i++)
            print_cell(k, i);
    }
    print_w();
    cursor();
}

const char * page::decoration(unsigned k, unsigned i)
{
    if (i % 8 == 0) return "| ";
    else if (i % 4 == 0) return ": ";
    else if (i % 2 == 0) return ". ";
    else return "  ";
}

void page::print_cell(unsigned k, unsigned i)
{
    ruler::cell * e = cell(k, i);
    mvprintw(k, x_of(i), "%s",
            e ? pitchname(e->p) : decoration(k, i));
}

void page::write_p(unsigned p)
{
    if ( ! p) a[c].e.erase(t);
    else {
        bool existed = cell(c, t);
        a[c].e[t].p = p;
        if ( ! existed) {
            a[c].e[t].q = a[c].saved_q;
            print_w();
        }
    }
    print_cell(c, t);
}

void page::cursor()
{
    mvchgat(c, x_of(t), 2, A_REVERSE, 0, NULL);
    refresh();
}

void page::no_cursor()
{
    mvchgat(c, x_of(t), 2, A_NORMAL, 0, NULL);
}

void page::move_t(int i)
{
    int u = t + i;
    if (u < 0) u += BE;
    else if (u >= BE) u -= BE;
    no_cursor();
    t = u;
    cursor();
    print_w();
}

void page::move_c(int i)
{
    int u = c + i;
    if (u < 0) u += a.size();
    else if (u >= (int)a.size()) u -= a.size();
    no_cursor();
    c = u;
    cursor();
    print_w();
}

void page::set_i() { a[c].i = s->consume_n(); print_i(c); }
void page::set_u() { a[c].u = s->consume_n(); print_u(c); }
void page::set_l() { a[c].l = s->consume_n(); print_l(c); }
void page::set_w() {
    ruler::cell * e = cell(c, t);
    if ( ! e) {
        write_p(12 * o);
        cursor();
        e = cell(c, t);
    }

    unsigned n = s->consume_n();
    if (n == 0) {
        e->q = a[c].saved_q;
        print_w();
    } else {
        div_t d = div(n, 10);
        unsigned i = d.rem;
        if (i == 0 || i == 9) return;
        e->q.a[i - 1] = d.quot % 100;
    }
    print_w();
    if (sound_entry y = play(c, t)) s->speak(y.s->build());
}

void page::save_w()
{
    ruler::cell * e = cell(c, t);
    if (e) a[c].saved_q = e->q;
}

void page::move_p(int j)
{
    for (unsigned i = 0; i < BE; i++)
        if (ruler::cell * e = cell(c, i)) {
            e->p += j;
            print_cell(c, i);
        }
}

void page::swap_r() {
    unsigned i = s->consume_n();
    if (i > s->nr()) return;
    unsigned k = (i) ? i - 1 : (c + 1) % a.size();
    if (k == c) return;
    std::swap(a[c], a[k]);
    for (unsigned i = 0; i < BE; i++)
        if (cell(c, i) || cell(k, i)) {
            print_cell(c, i);
            print_cell(k, i);
        }
    print_i(c); print_i(k);
    print_u(c); print_u(k);
    print_l(c); print_l(k);
    no_cursor();
    c = k;
    cursor();
}

void page::handle(int ch)
{
    if (ch == ' ') {
        write_p(0);
        move_t(1);
    } else if (unsigned p = scalekey(ch)) {
        if (unsigned n = s->consume_n()) o = std::min(n, 8u);
        write_p(p + 12 * o);
        if ( ! s->o->is_effect(a[c].i))
            if (sound_entry y = play(c, t)) s->speak(y.s->build());
        move_t(1);
    } else if (ch == 'r') {
        s->store(s->compile(0, h + 1));
    }
    else if (ch == 'i') set_i();
    else if (ch == 'u') set_u();
    else if (ch == 'l') set_l();
    else if (ch == 'w') set_w();
    else if (ch == 's') save_w();
    else if (ch == '<') move_p(-1);
    else if (ch == '>') move_p(1);
    else if (ch == '.') swap_r();
    else if (ch == KEY_LEFT) move_t(-1);
    else if (ch == KEY_RIGHT) move_t(1);
    else if (ch == KEY_UP) move_c(-1);
    else if (ch == KEY_DOWN) move_c(1);
    else if (ch == KEY_RESIZE) draw();
}

