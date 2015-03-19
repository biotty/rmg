//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "builders.hpp"
#include "generators.hpp"
#include "math.hpp"
#include <algorithm>

builder::~builder() {}

sound::sound(mv_ptr s, bu_ptr && b) : s(s), b(std::move(b)) {};

ug_ptr sound::build() { return U<multiply>(U<pulse>(s), b->build()); }

attack::attack(double h, double y1, double t, bu_ptr && w)
    : a(P<punctual>(0, y1))
    , s(P<stroke>(a, h, t))
    , w(U<sound>(s, std::move(w)))
{}

ug_ptr attack::build() { return w->build(); }

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
    : f(f), t(t), w(w), n(n)
{}

ug_ptr chorus::build()
{
    const double k = 1 / log2(n);
    mg_ptr s = U<sum>();
    for (unsigned i=0; i<n; i++) {
        const double m = (2 * i - double(n)) / n;
        en_ptr h = P<added>(f->e, P<scaled>(t, m));
        ug_ptr g = wave(P<still>(h->y(0)), w).build();
        record * r = new record(*g, P<movement>(P<inverted>(h), f->s));
        std::vector<double> & w = r->buffer_.w;
        std::rotate(w.begin(),
                w.begin() + unsigned(rnd(0, w.size())),
                w.end());
        s->c(U<periodic>(pg_ptr(r)), k);
    }
    return std::move(s);
}

cross::cross(bu_ptr && a, bu_ptr && b, mv_ptr c)
    : a(std::move(a)), b(std::move(b)), c(c)
{}

ug_ptr cross::build()
{
    ug_ptr wa = U<hung>(c);
    ug_ptr wb = U<hung>(P<movement>(P<subtracted>(P<constant>(1), c->e), c->s));
    mg_ptr mx = U<sum>();
    mx->c(U<multiply>(std::move(wa), a->build()), 1);
    mx->c(U<multiply>(std::move(wb), b->build()), 1);
    return std::move(mx);
}

fm::fm(bu_ptr && m, mv_ptr i, mv_ptr f) : m(std::move(m)), i(i), f(f) {}

ug_ptr fm::build()
{
    return U<modulation>(U<multiply>(U<pulse>(i), m->build()),
            U<sine>(rnd(0, 1)), f);
}

karpluss_strong::karpluss_strong(mv_ptr f, double a, double b)
    : f(f), a(a), b(b)
{}

ug_ptr karpluss_strong::build()
{
    noise n(1);
    return U<periodic>(U<karpluss>(P<strong>(a, b), n,
                P<movement>(P<inverted>(f->e), f->s)));
}

timed_filter::timed_filter(bs_ptr i, fl_ptr l, double t)
    : i(i), l(l), t(t)
{}

ug_ptr timed_filter::build()
{
    return U<timed>(U<filtration>(i->build(), l), t);
}

score::event::event(bs_ptr b, double t, unsigned i)
    : b(b), t(t), i(i)
{}

bool score::event::operator<(event const & e) const
{
    return t < e.t || (t == e.t && i < e.i);
}

score::score() : i() {}

void score::add(double t, bs_ptr b)
{
    v.emplace_hint(v.cend(), b, t, i++);
}

ug_ptr score::build()
{
    mg_ptr m = U<delayed_sum>();
    for (auto & e : v) m->c(U<lazy>(e.b), e.t);
    return U<limiter>(std::move(m));
}

adder::adder() : w(1) {}

ug_ptr adder::build()
{
    mg_ptr s = U<sum>();
    for (auto & p : v) s->c(p->build(), w);
    return std::move(s);
}

trunk::trunk(bu_ptr && input)
    : input(std::move(input))
    , a(new adder), n()
{}

void trunk::branch(bu_ptr && b) { a->v.push_back(std::move(b)); }

ug_ptr trunk::build_leaf()
{
    if (a) throw "build leaf on non-concluded trunk";

    if ( ! g) g = P<ncopy>(n, input->build());
    return U<wrapshared>(g);
}

bu_ptr trunk::conclude()
{
    n = a->v.size();
    a->w = 1 / log2(n);
    bu_ptr r = bu_ptr(a);
    a = nullptr;
    return r;
}

leaf::leaf(ts_ptr trunk_) : t(trunk_) {}
ug_ptr leaf::build() { return t->build_leaf(); }
