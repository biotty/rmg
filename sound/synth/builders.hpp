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

    sound(bu_ptr && b, double t, en_ptr e);
    ug_ptr build();
};

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
    double t;
    en_ptr c;

    cross(bu_ptr && a, bu_ptr && b, double t, en_ptr c);
    ug_ptr build();
};

struct harmonics : builder
{
    en_ptr freq;
    en_ptr e;
    double m;
    double even;
    double a(double f, unsigned i);
    double p(double b);

    harmonics(en_ptr freq, double m, en_ptr e, double even);
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
    double t;
    en_ptr i;
    en_ptr carrier_freq;

    fm(bu_ptr && m, double t, en_ptr i, en_ptr carrier_freq);
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
    bu_ptr i;
    fl_ptr l;
    double t;

    timed_filter(bu_ptr i, fl_ptr l, double t);
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

#endif
