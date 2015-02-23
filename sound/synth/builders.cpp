//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "builders.hpp"
#include "generators.hpp"
#include "math.hpp"
#include <algorithm>

builder::~builder() {}

sound::sound(mv_ptr s, bu_ptr c) : s(s), c(c) {};

ug_ptr sound::build()
{
    return U<multiply>(U<pulse>(s), c->build());
}

attack::attack(double h, double y1, double t, bu_ptr c)
    : a(P<punctual>(0, y1))
    , s(P<stroke>(a, h, t))
    , w(P<sound>(s, c))
{}

ug_ptr attack::build() {
    return w->build();
}

wave::wave(mv_ptr f, en_ptr e) : f(f), e(e) {}

ug_ptr wave::build()
{
    mv_ptr p = P<movement>(P<inverted>(f->e), f->s);
    pulse w(P<movement>(e, p->z(0)));
    return U<periodic>(U<record>(w, p));
}

harmonics::harmonics(mv_ptr f, en_ptr e, unsigned n, double ow)
    : f(f), e(e), n(n), odd(.5 * (1 + ow)), even(1 - odd)
{
    const double a = 1 / std::max(odd, even);
    odd *= a;
    even *= a;
}

double harmonics::w(unsigned i)
{
    if (i) return (i&1) ? even : odd;
    else return 1;
}

double harmonics::a(unsigned i)
{
    double x = i /(double) n;
    const double y = e->y(x);
    return w(i) * y * y;
}

double harmonics::p()
{
    double s = 0;
    for (unsigned i=0; i<n; i++) s += a(i);
    return sqrt(s);
}

ug_ptr harmonics::build()
{
    const double k = 1 / p();
    const double b = f->z(0);
    sum s;
    for (unsigned i=0; i<n; i++) {
        const double f = b * (i + 1);
        if (f * 3 > SR) break;
        en_ptr w = P<sine>(rnd(0, 1));
        s.c(wave(P<still>(f), w).build(), a(i) * k);
    }
    return U<periodic>(U<record>(s, P<movement>(P<inverted>(f->e), f->s)));
};

chorus::chorus(mv_ptr f, en_ptr t, en_ptr w, unsigned n)
    : f(f), t(t), w(w), n(n) {}

ug_ptr chorus::build()
{
    const double k = 1 / log2(n);
    mg_ptr s = U<sum>();
    for (unsigned i=0; i<n; i++) {
        const double m = (2 * i - double(n)) / n;
        en_ptr h = P<added>(f->e, P<scaled>(t, m));
        ug_ptr g = wave(P<still>(h->y(0)), w).build();
        record * r = new record(*g, P<movement>(P<inverted>(h), f->s));
        std::vector<double> & w = r->b.w;
        std::rotate(w.begin(),
                w.begin() + unsigned(rnd(0, w.size())),
                w.end());
        s->c(U<periodic>(pg_ptr(r)), k);
    }
    return std::move(s);
}

cross::cross(bu_ptr a, bu_ptr b, mv_ptr c) : a(a), b(b), c(c) {}

ug_ptr cross::build()
{
    if (c->z(c->s) <= .9) throw 1; //see that user took care of ending c high
                                   //should actually be very close to 1
                                   //because wb will step to 0 at c->s as it's
                                   //an envelope's irect values (to be !more at c->s
                                   // -- but the built ug will never be !more anyway,
                                   //so we could solve the problem of disconituity by
                                   //simply making wb a P<hung> instead of P<pulse>
                                   //as well.  then remove this if ... throw line.
    ug_ptr wa = U<hung>(c);
    ug_ptr wb = U<hung>(P<movement>(P<subtracted>(P<constant>(1), c->e), c->s));
    mg_ptr mx = U<sum>();
    mx->c(U<multiply>(std::move(wa), a->build()), 1);
    mx->c(U<multiply>(std::move(wb), b->build()), 1);
    return std::move(mx);
}

fm::fm(bu_ptr m, mv_ptr i, mv_ptr f) : m(m), i(i), f(f) {}

ug_ptr fm::build()
{
    return U<modulation>(U<multiply>(U<pulse>(i), m->build()),
            U<sine>(rnd(0, 1)), f);
}

karpluss_strong::karpluss_strong(mv_ptr f, double a, double b) : f(f), a(a), b(b) {}

ug_ptr karpluss_strong::build()
{
    noise n(1);
    return U<periodic>(U<karpluss>(P<strong>(a, b), n,
                P<movement>(P<inverted>(f->e), f->s)));
}

timed_filter::timed_filter(bu_ptr i, fl_ptr l, double t) : i(i), l(l), t(t) {}

ug_ptr timed_filter::build()
{
    return U<timed>(U<filtration>(i->build(), l), t);
}

bool score::event::operator<(event const & e) const
{ return t < e.t || (t == e.t && i < e.i); }

score::score() : i() {}

void score::add(double t, bu_ptr c) { v.insert({c, t, i++}); }

ug_ptr score::build()
{
    mg_ptr m = U<delayed_sum>();
    for (auto & e : v) m->c(U<lazy>(e.c), e.t);
    return U<limiter>(std::move(m));
}

adder::adder() : w(1) {}

ug_ptr adder::build()
{
    mg_ptr s = U<sum>();
    for (auto & p : v) s->c(p->build(), w);
    return std::move(s);
}
