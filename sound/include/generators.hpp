//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef GENERATORS_HPP
#define GENERATORS_HPP

#include "builderdecl.hpp"
#include "envelopes.hpp"
#include "filters.hpp"
#include "unit.hpp"
#include <set>

struct generators_global
{
    unsigned sr;
    generators_global();
};

extern generators_global ug_global;

#define SR ug_global.sr

struct infinite : generator
{
    bool more();
};

struct silence : infinite
{
    void generate(unit & u);
};

struct noise : infinite
{
    double a;

    noise(double a);
    void generate(unit & u);
};

struct pulse : generator
{
    mv_ptr m;
    unsigned k;
    double start();
    double step();
    double at();

    pulse(mv_ptr m);
    void generate(unit & u);
    bool more();
};

struct hung : pulse
{
    hung(mv_ptr m);
    void generate(unit & u);
    bool more();
};

struct period : generator
{
    virtual unsigned size() = 0;
    virtual void reset() = 0;
};

typedef std::shared_ptr<period> pg_ptr;

struct record : period
{
    period_buffer b;
    mv_ptr duration;
    unsigned c;
    double t;
    bool more();
    unsigned size();
    void reset();
    record(ug_ptr g, mv_ptr duration);
    void generate(unit & u);
};

struct karpluss : record
{
    fl_ptr l;

    karpluss(fl_ptr l, ug_ptr g, mv_ptr duration);
    void reset();
};

struct periodic : infinite
{
    struct buffer
    {
        unsigned head;
        unsigned tail;
        double a[2*SU];
        unsigned n();

        buffer();
        unsigned post_incr(unsigned & i);
        void put(double y);
        double get();
    };

    pg_ptr g;
    std::unique_ptr<buffer> carry;
    void append(unit & u, unsigned n);
    void shift(unit & u);

    periodic(pg_ptr g);
    void generate(unit & u);
};

struct multiply : generator
{
    ug_ptr a;
    ug_ptr b;

    multiply(ug_ptr a, ug_ptr b);
    void generate(unit & u);
    bool more();
};

struct mix : generator
{
    virtual void c(ug_ptr g, double p) = 0;
};

typedef std::shared_ptr<mix> mg_ptr;

template<typename T>
struct weighted
{
    T e;
    double w;
};

struct product : mix
{
    std::vector<weighted<ug_ptr>> s;
    bool anymore;

    product();
    void c(ug_ptr g, double w);
    void generate(unit & u);
    bool more();
};

struct sum : mix
{
    std::vector<weighted<ug_ptr>> s;
    bool anymore;

    sum();
    void c(ug_ptr g, double w);
    void generate(unit & u);
    bool more();
};

struct modulation : infinite
{
    ug_ptr m;
    en_ptr c;
    mv_ptr f;
    double x;
    double t;

    modulation(ug_ptr m, en_ptr c, mv_ptr f);
    void generate(unit & u);
};

struct delayed_sum : mix
{
    struct entry
    {
        double t;
        ug_ptr g;
        unsigned offset;

        entry(double t, ug_ptr g);
    };

    struct couple
    {
        unit a;
        unit b;
        unsigned c;

        couple();
        void add(unsigned h, unit & u);
        void flush(unit & u);
        bool carry();
    };

    std::vector<entry> entries;
    std::unique_ptr<couple> v;
    bool pending;
    unsigned k;

    delayed_sum();
    void c(ug_ptr g, double t);
    void generate(unit & u);
    bool more();
};

struct generator_creation
{
    virtual ug_ptr build();
};

struct lazy : generator
{
    ug_ptr g;
    bu_ptr c;

    lazy(bu_ptr c);
    void generate(unit & u);
    bool more();
};

struct limiter : generator
{
    ug_ptr z;
    unit v;
    bool b;
    bool q;
    double i;
    double s;
    double g;
    void out(unit & u, double t);
    
    limiter(ug_ptr z);
    void generate(unit & u);
    bool more();
};

struct timed : generator
{
    ug_ptr g;
    unsigned n;
    unsigned k;

    timed(ug_ptr g, double t);
    void generate(unit & u);
    bool more();
};

struct filtration : infinite
{
    ug_ptr g;
    fl_ptr l;
    filtration(ug_ptr g, fl_ptr l);
    void generate(unit & u);
};

#endif

