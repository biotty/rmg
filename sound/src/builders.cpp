#include "builders.hpp"

#include "generators.hpp"
#include "math.hpp"
#include <algorithm>

builders_global cr_global;

builders_global::builders_global() : psi()
{
    phases.push_back(0);
    for (unsigned i=1; i<64; i++) phases.push_back(rnd(0, 1));
}

double builders_global::phase(unsigned i)
{
    return phases[(i + psi) % phases.size()];
}

psi::psi(bu_ptr c) : c(c) {}

ug_ptr psi::build() { cr_global.psi++; return c->build(); }

sound::sound(mv_ptr s, bu_ptr c) : s(s), c(c) {};

ug_ptr sound::build()
{
    return P<multiply>(P<pulse>(s), c->build());
}

attack::attack(double h, double y1, double t, bu_ptr c)
    : a(P<punctual>(0, y1))
    , s(P<stroke>(a, h, t))
    , w(P<psi>(P<sound>(s, c)))
{}

ug_ptr attack::build() { return w->build(); }

wave::wave(mv_ptr f, en_ptr e) : f(f), e(e) {}

ug_ptr wave::build()
{
    mv_ptr p = P<movement>(P<inverted>(f->e), f->s);
    ug_ptr w = P<pulse>(P<movement>(e, p->z(0)));
    pg_ptr g = P<record>(w, p);
    return P<periodic>(g);
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
    mg_ptr s = P<sum>();
    for (unsigned i=0; i<n; i++) {
        const double f = b * (i + 1);
        if (f * 3 > ug_global.sr) break;
        en_ptr w = P<sine>(cr_global.phase(i));
        s->c(wave(P<still>(f), w).build(), a(i) * k);
    }
    pg_ptr r = P<record>(s, P<movement>(P<inverted>(f->e), f->s));
    return P<periodic>(r);
};

chorus::chorus(mv_ptr f, en_ptr t, en_ptr w, unsigned n)
    : f(f), t(t), w(w), n(n) {}

ug_ptr chorus::build()
{
    const double k = 1 / log2(n);
    mg_ptr s = P<sum>();
    for (unsigned i=0; i<n; i++) {
        const double m = (2 * i - double(n)) / n;
        en_ptr h = P<added>(f->e, P<scaled>(t, m));
        ug_ptr g = wave(P<still>(h->y(0)), w).build();
        record * r = new record(g, P<movement>(P<inverted>(h), f->s));
        pg_ptr p(r);
        s->c(P<periodic>(p), k);
        std::vector<double> & w = r->b.w;
        std::rotate(w.begin(),
                w.begin() + unsigned(w.size() * cr_global.phase(i)),
                w.end());
    }
    return s;
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
    ug_ptr wa = P<hung>(c);
    ug_ptr wb = P<pulse>(P<movement>(P<subtracted>(P<constant>(1), c->e), c->s));
    mg_ptr mx = P<sum>();
    mx->c(P<multiply>(wa, a->build()), 1);
    mx->c(P<multiply>(wb, b->build()), 1);
    return mx;
}

fm::fm(bu_ptr m, mv_ptr i, mv_ptr f) : m(m), i(i), f(f) {}

ug_ptr fm::build()
{
    return P<modulation>(P<multiply>(P<pulse>(i), m->build()),
            P<sine>(cr_global.phase(0)), f);
}

karpluss_strong::karpluss_strong(mv_ptr f, double a, double b) : f(f), a(a), b(b) {}

ug_ptr karpluss_strong::build()
{
    pg_ptr r = P<karpluss>(P<strong>(a, b), P<noise>(1),
            P<movement>(P<inverted>(f->e), f->s));
    return P<periodic>(r);
}

timed_filter::timed_filter(bu_ptr i, fl_ptr l, double t) : i(i), l(l), t(t) {}

ug_ptr timed_filter::build()
{
    return P<timed>(P<filtration>(i->build(), l), t);
}

bool score::event::operator<(event const & e) const
{ return t < e.t || (t == e.t && i < e.i); }

score::score() : i() {}

void score::add(double t, bu_ptr c) { v.insert({c, t, i++}); }

ug_ptr score::build()
{
    mg_ptr m = P<delayed_sum>();
    for (auto e : v) m->c(P<lazy>(e.c), e.t);
    return P<limiter>(m);
}

adder::adder() : w(1) {}

ug_ptr adder::build()
{
    mg_ptr s = P<sum>();
    for (auto p : v) s->c(p->build(), w);
    return s;
}
