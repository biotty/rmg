//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef FILTERS_HPP
#define FILTERS_HPP

#include "ringbuf.hpp"
#include "envelopes.hpp"

#include <memory>
#include <vector>

struct filter
{
    virtual double shift(double y) = 0;
};

typedef std::shared_ptr<filter> fl_ptr;

class period_buffer
{
    double f;
public:
    std::vector<double> w;

    period_buffer(unsigned n);
    void cycle(double s);
};

struct as_is : filter
{
    double shift(double y);
};

struct strong : filter
{
    double a;
    double b;
    double w;
    double shift(double y);

    strong(double a, double b);
};

struct delay_network
{
    unsigned i;
    period_buffer b;
    fl_ptr l;
    en_ptr s;
    double x_cycle;

    delay_network(fl_ptr l, en_ptr s);
    void step(double z);
    double current();
};

class control_clock
{
    unsigned a;
    unsigned b;
    unsigned i;
public:
    double x;

    control_clock(unsigned a, unsigned b = 0);
    bool tick();
};

struct feed : filter
{
    bool back;
    control_clock c;
    delay_network d;
    en_ptr amount;
    en_ptr delay;
    double g;

    feed(fl_ptr l, en_ptr amount, en_ptr delay, bool back = false);
    double shift(double y);
};

struct feedback : feed { feedback(fl_ptr l, en_ptr amount, en_ptr delay); };

struct biquad : filter
{
    double w1, w2;
    double b0, b1, b2, a1, a2;
    control_clock cc;
    struct control
    {
        en_ptr b0;
        en_ptr b1;
        en_ptr b2;
        en_ptr a1;
        en_ptr a2;
    };
    control fc;

    biquad(control fc);
    double shift(double y);
};

#endif
