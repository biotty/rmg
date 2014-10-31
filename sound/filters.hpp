#ifndef FILTERS_HPP
#define FILTERS_HPP

#include "ringbuf.hpp"
#include "envelopes.hpp"

#include <memory>
#include <vector>

struct filters_global
{
    unsigned sr;
    filters_global();
};

extern filters_global fl_global;

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
    double s;

    delay_network(fl_ptr l, double s);
    void step(double z);
    double current();
};

class control_clock
{
    unsigned i;
    double x;
public:
    control_clock();
    void tick(std::function<void(double)> f);
};

struct feed : filter
{
    bool back;
    control_clock c;
    delay_network d;
    mv_ptr f;
    double g;
    en_ptr s;

    feed(fl_ptr l, mv_ptr f, en_ptr s);
    double shift(double y);
};

struct feedback : feed { feedback(fl_ptr l, mv_ptr f, en_ptr s); };

#endif

