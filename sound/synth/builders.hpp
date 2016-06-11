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
