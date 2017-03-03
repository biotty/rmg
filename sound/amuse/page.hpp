//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef PAGE_HPP
#define PAGE_HPP

#include "orchestra.hpp"

#include <map>

struct screen;

#define BE 32

struct page
{
    screen * s;
    unsigned c;
    unsigned t;
    unsigned h;
    unsigned o;
    unsigned m;
    struct ruler {
        unsigned i;
        unsigned l;
        unsigned u;
        struct cell {
            unsigned p;
            cell();
        };
        std::map<unsigned, cell> e;
        ruler();
    };
    std::vector<ruler> a;
    page(screen * s, unsigned h);
    ruler::cell * cell(unsigned k, unsigned i);
    unsigned pitch(unsigned k, unsigned i);
    sound_entry play(unsigned k, unsigned i);
    int x_of(unsigned i);
    void print_i(int k);
    void print_u(int k);
    void print_l(int k);
    void draw();
    const char * decoration(unsigned i);
    void print_cell(unsigned k, unsigned i);
    void write_p(unsigned p);
    void cursor();
    void no_cursor();
    void move_t(int i);
    void move_c(int i);
    void set_i();
    void set_u();
    void set_l();
    void move_p(int j);
    void swap_r();
    void handle(int e);
};

#endif

