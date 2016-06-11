//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "builders.hpp"
#include "generators.hpp"
#include "math.hpp"
#include <algorithm>

builder::~builder() {}

// builders raison-d'etre: enable mem-consuming builders to not be constructed
// until needed.  the lazy construction is done in score, so that larger constructions
// can be formed but the data and buffers for the generators are only present when
// generated and then destructed.  it seems they could be generators, and take care
// of the lazy data and destruction of inputs themselves.  then the technical lazy
// generator would not be needed, and replaced by some internal helpers.
// there is however a conceptual difference, as builders combine other generators only
// and do not manipulate the data themselves.  with c++17 move-capture support
// in lambdas, a generic builder can be provided, where the user specified the delayed
// generator-setup at the call-site.  this generic builder could still also itself
// be a generator.  there might be a pre-c++17 trick using std::bind and references
// to the unique-pointer providing a movable std::function.

sound::sound(en_ptr e, double t, bu_ptr && b)
    : e(e), t(t), b(std::move(b))
{}

ug_ptr sound::build()
{
    return U<genv>(b->build(), t, e);
}

attack::attack(double h, double y1, double t, bu_ptr && w)
    : a(P<punctual>(0, y1))
    , s(make_stroke(a, h, t))
    , w(U<sound>(s, t, std::move(w)))
{}

ug_ptr attack::build() { return w->build(); }

trapesoid::trapesoid(double h, double y1, double t, bu_ptr && w)
    : a(P<punctual>(0, 0))
    , s(P<stretched>(a, t))
    , w(U<sound>(s, t, std::move(w)))
{
    const double d = h / t;
    a->p(0 + d, y1);
    a->p(1 - d, y1);
}

ug_ptr trapesoid::build() { return w->build(); }

wave::wave(en_ptr freq, en_ptr e) : freq(freq), e(e) {}

ug_ptr wave::build()
{
    en_ptr period = P<shaped>(freq, inverts());
    const double z = period->y(0);
    gent w(P<stretched>(e, z), z);
    return U<periodic>(U<record>(w, period));
}

cross::cross(bu_ptr && a, bu_ptr && b, double t, en_ptr c)
    : a(std::move(a)), b(std::move(b)), t(t), c(c)
{}

ug_ptr cross::build()
{
    mg_ptr mx = U<sum>();
    en_ptr d = P<shaped>(c, [](double x){ return 1 - x; });
    mx->c(U<genv>(a->build(), t, c, true), 1);
    mx->c(U<genv>(b->build(), t, d), 1);
    return std::move(mx);
}

harmonics::harmonics(en_ptr freq, en_ptr e, double ow, double m)
    : freq(freq), e(e), m(m), odd(.5 * (1 + ow)), even(1 - odd)
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

double harmonics::a(double f)
{
    const double x = f / m;
    const double y = e->y(x);
    return y * y;
}

double harmonics::p(double b)
{
    double s = 0;
    for (unsigned i=0; ; i+=2) {
        const double f = b * (i + 1);
        if (f >= m) break;
        s += a(f);
    }
    return sqrt(s);
}

ug_ptr harmonics::build()
{
    const double b = freq->y(0);
    const double k = 1 / p(b);
    sum s;
    for (unsigned i=0; ; i++) {
        const double f = b * (i + 1);
        if (f >= m) break;
        // quality: instead of 0, make possible to phase-shift all
        //          but having them phase-synced to some t.
        //          this is not realized by having non-zero as of now
        //          -- figure out what semantic of sine argument
        //          (the phase-sync is consistent with wave-phenomena)
        //          also, make shure odd/even semantics is sane; specifically
        //          not permit even-only if not physically occurs
        s.c(wave(P<constant>(f), P<sine>(0)).build(),
                a(f) * w(i) * k);
    }
    return U<periodic>(U<record>(s, P<shaped>(freq, inverts())));
};

chorus::chorus(en_ptr freq, en_ptr t, en_ptr w, unsigned n)
    : freq(freq), t(t), w(w), n(n)
{}

ug_ptr chorus::build()
{
    const double k = 1 / log2(n);
    mg_ptr s = U<sum>();
    for (unsigned i=0; i<n; i++) {
        const double m = (2 * i - double(n)) / n;
        en_ptr h = P<added>(freq, P<shaped>(t, scales(m)));
        ug_ptr g = wave(P<constant>(h->y(0)), w).build();
        record * r = new record(*g, P<shaped>(h, inverts()));
        std::vector<double> & w = r->buffer_.w;
        std::rotate(w.begin(),
                w.begin() + unsigned(rnd(0, w.size())),
                w.end()); // note: ensure not phase-sync on start
        s->c(U<periodic>(pg_ptr(r)), k);
    }
    return std::move(s);
}

am::am(bu_ptr && a, bu_ptr && b) : a(std::move(a)), b(std::move(b)) {}

ug_ptr am::build() { return U<multiply>(a->build(), b->build()); }

fm::fm(bu_ptr && m, double t, en_ptr i, en_ptr carrier_freq)
    : m(std::move(m)), t(t), i(i), carrier_freq(carrier_freq)
{}

ug_ptr fm::build()
{
    return U<modulation>(
            U<genv>(m->build(), t, i),  // obs: this is sound().  let caller provide it
            U<sine>(rnd(0, 1)), carrier_freq);
}

karpluss_strong::karpluss_strong(en_ptr freq, double a, double b)
    : freq(freq), a(a), b(b)
{}

ug_ptr karpluss_strong::build()
{
    noise n(1);
    return U<periodic>(U<karpluss>(P<strong>(a, b),
                n, P<shaped>(freq, inverts())));
}

timed_filter::timed_filter(bu_ptr i, fl_ptr l, double t)
    : i(std::move(i)), l(l), t(t)
{}

ug_ptr timed_filter::build()
{
    return U<filtration>(i->build(), l, t);
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
