//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef GENERATORS_HPP
#define GENERATORS_HPP

#include "envelopes.hpp"
#include "filters.hpp"
#include "unit.hpp"
#include <set>

struct infinite : generator { bool more(); };

struct silence : infinite { void generate(unit & u); };

struct noise : infinite
{
    double amplitude_;

    noise(double amplitude);
    void generate(unit & u);
};

struct gen : infinite
{
    en_ptr e;
    unsigned units_generated_;

    gen(en_ptr e);
    void generate(unit & u);
};

struct gent : gen // note: reuses gen but not infinite
{
    double s;

    gent(en_ptr e, double s);
    bool more();
};

struct filtration : infinite
{
    ug_ptr g;
    fl_ptr l;

    filtration(ug_ptr && g, fl_ptr l);
    void generate(unit & u);
};

struct timed : generator
{
    ug_ptr generator_;
    unsigned units_to_generate_;
    unsigned units_generated_;

    timed(ug_ptr && g, double t);
    void generate(unit & u);
    bool more();
};

struct period : generator
{
    double duration;

    virtual unsigned size() = 0;
    virtual void reset() = 0;
};

typedef std::unique_ptr<period> pg_ptr;

struct record : period
{
    period_buffer buffer_;
    en_ptr duration_;
    unsigned samples_generated_;
    double t_reset;
    bool more();
    unsigned size();
    void reset();

    record(generator & g, en_ptr duration);
    void generate(unit & u);
};

struct karpluss : record
{
    fl_ptr filter_;

    karpluss(fl_ptr l, generator & g, en_ptr duration);
    void reset();
};

struct periodic : infinite
{
    struct carry_buffer
    {
        unsigned head;
        unsigned tail;
        double a[2 * unit::size];
        unsigned n();

        carry_buffer();
        unsigned post_incr(unsigned & i);
        void put(double y);
        double get();
    };
    pg_ptr g_;
    std::unique_ptr<carry_buffer> carry_;
    void append(unit & u, unsigned n);
    void shift(unit & u);

    periodic(pg_ptr && g);
    void generate(unit & u);
};

struct multiply : generator
{
    ug_ptr a_;
    ug_ptr b_;

    multiply(ug_ptr && a, ug_ptr && b);
    void generate(unit & u);
    bool more();
};

struct mix : generator
{
    virtual void c(ug_ptr && g, double p) = 0;
};

typedef std::unique_ptr<mix> mg_ptr;

struct weighted_transform : mix
{
    struct component
    {
        ug_ptr e;
        double w;

        component(ug_ptr && e, double w) : e(std::move(e)), w(w) {}
    };
    std::vector<component> s_;
    virtual void f(unit & accumulator, unit & u, double w) = 0;
    const double init_;
    bool anymore_;

    weighted_transform(double i);
    void c(ug_ptr && g, double w);
    void generate(unit & u);
    bool more();
};

// cleanup:
//   remove product. (unused)
//   remove mg_ptr. (specific class when used)
//   remove weight on sum. (scale output instead, or individuals if needed)
//   remove weighted_transform. (abstraction nor weight-component needed)
//   remove unit.add and unit.mul. (unused)

struct sum : weighted_transform
{
    sum();
    void f(unit & accumulator, unit & u, double w);
};

struct product : weighted_transform
{
    product();
    void f(unit & accumulator, unit & u, double w);
};

struct modulation : infinite
{
    ug_ptr modulator_;
    en_ptr carrier_;
    en_ptr freq_;
    double x_;
    double t_generated_;

    modulation(ug_ptr && modulator, en_ptr carrier, en_ptr freq);
    void generate(unit & u);
};

struct delayed_sum : mix
{
    struct term
    {
        double t;
        ug_ptr g;
        unsigned offset;

        term(double t, ug_ptr && g);
    };

    struct glue_buffer
    {
        unit a;
        unit b;
        unsigned c;

        glue_buffer();
        void add(unsigned h, unit & u);
        void flush(unit & u);
        bool more_to_flush();
    };

    std::vector<term> terms_;
    std::unique_ptr<glue_buffer> buffer_;
    bool pending_;
    unsigned units_generated_;

    delayed_sum();
    void c(ug_ptr && g, double t);
    void generate(unit & u);
    bool more();
};

struct lazy : generator
{
    ug_ptr generator_;
    bs_ptr builder_;

    lazy(bs_ptr b);
    void generate(unit & u);
    bool more();
};

struct limiter : generator
{
    ug_ptr generator_;
    unit buffer_unit_;
    bool started_;
    bool quit_;
    double increment_;
    double s_;
    double glider_;
    void out(unit & u, double target);
    
    limiter(ug_ptr && z);
    void generate(unit & u);
    bool more();
};

struct ncopy : generator
{
    int i;
    int n;
    unit v;
    ug_ptr g;

    ncopy(int n, ug_ptr && g);
    void generate(unit & u);
    bool more();
};

struct wrapshared : generator
{
    std::shared_ptr<generator> g;

    wrapshared(std::shared_ptr<generator> g);
    void generate(unit & u);
    bool more();
};

struct inter : generator
{
    std::shared_ptr<generator> l;
    std::shared_ptr<generator> r;
    unsigned n;
    unsigned j;
    unit lu;
    unit ru;

    inter(ug_ptr && l, ug_ptr && r);
    void generate(unit & u);
    bool more();
};

#endif
