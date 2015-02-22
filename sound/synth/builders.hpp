//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef BUILDERS_HPP
#define BUILDERS_HPP

#include "builderdecl.hpp"
#include "envelopes.hpp"
#include "filters.hpp"
#include <set>

struct sound : builder
{
    mv_ptr s;
    bu_ptr c;

    sound(mv_ptr s, bu_ptr c);
    ug_ptr build();
};

struct attack : builder
{
    pe_ptr a;
    mv_ptr s;
    bu_ptr w;

    attack(double h, double y1, double t, bu_ptr c);
    ug_ptr build();
};

typedef std::shared_ptr<attack> at_ptr;

struct wave : builder
{
    mv_ptr f;
    en_ptr e;

    wave(mv_ptr f, en_ptr e);
    ug_ptr build();
};

struct harmonics : builder
{
    mv_ptr f;
    en_ptr e;
    unsigned n;
    double odd;
    double even;
    double w(unsigned i);
    double a(unsigned i);
    double p();

    harmonics(mv_ptr f, en_ptr e, unsigned n, double ow);
    ug_ptr build();
};

struct chorus : builder
{
    mv_ptr f;
    en_ptr t;
    en_ptr w;
    unsigned n;

    chorus(mv_ptr f, en_ptr t, en_ptr w, unsigned n);
    ug_ptr build();
};

struct cross : builder
{
    bu_ptr a;
    bu_ptr b;
    mv_ptr c;

    cross(bu_ptr a, bu_ptr b, mv_ptr c);
    ug_ptr build();
};

struct fm : builder
{
    bu_ptr m;
    mv_ptr i;
    mv_ptr f;

    fm(bu_ptr m, mv_ptr i, mv_ptr f);
    ug_ptr build();
};

struct karpluss_strong : builder
{
    mv_ptr f;
    double a;
    double b;

    karpluss_strong(mv_ptr f, double a, double b);
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
        bu_ptr c;
        double t;
        unsigned i;
        bool operator<(event const & e) const;
    };
    unsigned i;
    std::set<event> v;
    void add(double t, bu_ptr c);

    score();
    ug_ptr build();
};

typedef std::shared_ptr<score> sc_ptr;

struct adder : builder
{
    std::vector<bu_ptr> v;
    double w;

    adder();
    ug_ptr build();
};


#endif

