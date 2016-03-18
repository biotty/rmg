//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef BUILDERS_HPP
#define BUILDERS_HPP

#include "envelopes.hpp"
#include "filters.hpp"
#include <set>

struct sound : builder
{
    en_ptr e;
    double t;
    bu_ptr b;

    sound(en_ptr e, double t, bu_ptr && b);
    ug_ptr build();
};

struct attack : builder
{
    pe_ptr a;
    en_ptr s;
    bu_ptr w;

    attack(double h, double y1, double t, bu_ptr && w);
    ug_ptr build();
};

struct trapesoid : builder
{   //(isosceles)
    pe_ptr a;
    en_ptr s;
    bu_ptr w;

    trapesoid(double h, double y1, double t, bu_ptr && w);
    ug_ptr build();
};

typedef std::unique_ptr<attack> at_ptr;

struct wave : builder
{
    en_ptr freq;
    en_ptr e; // one period [0,1) of wave

    wave(en_ptr freq, en_ptr e);
    ug_ptr build();
};

struct cross : builder
{
    bu_ptr a;
    bu_ptr b;
    en_ptr c;

    cross(bu_ptr && a, bu_ptr && b, en_ptr c);
    ug_ptr build();
};

struct harmonics : builder
{
    en_ptr freq;
    en_ptr e;
    double m;
    double odd;
    double even;
    double w(unsigned i);
    double a(double f);
    double p(double b);

    harmonics(en_ptr freq, en_ptr e, double ow, double m);
    ug_ptr build();
};

struct chorus : builder
{
    en_ptr freq;
    en_ptr t;
    en_ptr w;
    unsigned n;

    chorus(en_ptr freq, en_ptr t, en_ptr w, unsigned n);
    ug_ptr build();
};

struct am : builder
{
    bu_ptr a;
    bu_ptr b;

    am(bu_ptr && a, bu_ptr && b);
    ug_ptr build();
};

struct fm : builder
{
    bu_ptr m;
    en_ptr i;
    en_ptr carrier_freq;

    fm(bu_ptr && m, en_ptr i, en_ptr carrier_freq);
    ug_ptr build();
};

struct karpluss_strong : builder
{
    en_ptr freq;
    double a;
    double b;

    karpluss_strong(en_ptr freq, double a, double b);
    ug_ptr build();
};

struct timed_filter : builder
{
    bs_ptr i;
    fl_ptr l;
    double t;

    timed_filter(bs_ptr i, fl_ptr l, double t);
    ug_ptr build();
};

struct score : builder
{
    struct event
    {
        bs_ptr b;
        double t;
        unsigned i;

        event(bs_ptr b, double t, unsigned i);
        bool operator<(event const & e) const;
    };
    unsigned i;
    std::set<event> v;
    void add(double t, bs_ptr b);

    score();
    ug_ptr build();
};

typedef std::unique_ptr<score> sc_ptr;

struct adder : builder
{
    std::vector<bu_ptr> v;
    double w;

    adder();
    ug_ptr build();
};

// consider: remove (stereo does it by hand and it seems fine,
//           so fmix may also do it that way -- try it out
struct trunk
{
    bu_ptr input;
    std::shared_ptr<generator> g;
    adder * a;
    int n;
    ug_ptr build_leaf();

    trunk(bu_ptr && input);
    void branch(bu_ptr && b);
    bu_ptr conclude();

    typedef std::shared_ptr<trunk> ptr;

    struct leaf : builder
    {
        ptr t;

        leaf(ptr trunk_);
        ug_ptr build();
    };
};

#endif
