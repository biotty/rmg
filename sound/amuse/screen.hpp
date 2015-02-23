//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef SCREEN_HPP
#define SCREEN_HPP

#include "unitfwd.hpp"

struct orchestra;

struct sum;
struct book;
class screen
{
    bool z;
    bool r;
    ug_ptr lim;
    sum * g_ptr();
public:
    void store(generator & c);
    void speak(ug_ptr && c);
    void shut();

    bool editing();
    bool running();
    void start();
    void stop();
    void print_r();
    void write_r(bool i);

    unsigned n;
    unsigned consume_n();

    unsigned m;
    void print_m();
    void write_m(unsigned i);
    double duration();

    book * k;
    ug_ptr compile(unsigned p, unsigned q);

    orchestra * o;
    void edit();
    bool eager();
    generator * mastered();

    screen();
    ~screen();

    unsigned nr();
};

#endif

