//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "generators.hpp"
#include "math.hpp"
#include <algorithm>

namespace {

template<typename T>
void fill(generator & g, T ptr, unsigned n)
{
    const unsigned b = n / SU;
    for (unsigned k=0; k!=b; k++) {
        unit u;
        g.generate(u);
        std::copy(u.y, u.y + SU, ptr + k * SU);
    }
    if (unsigned r = n % SU) {
        unit v;
        g.generate(v);
        std::copy(v.y, v.y + r, ptr + b * SU);
    }
}

inline void mod1inc(double & x, double d)
{
    if (fabs(d) >= 1) throw 1;
    x += d;
    if (x >= 1) x -= 1;
    else if (x < 0) x += 1;
}

inline double famp(double z) { return z * z; }

bool nomore(weighted<ug_ptr> & c) { return ! c.e->more(); }

void prune(std::vector<weighted<ug_ptr>> & s)
{
    s.erase(std::remove_if(s.begin(), s.end(), nomore), s.end());
}

}

generator::~generator() {}

bool infinite::more() { return true; }

void silence::generate(unit & u) { u.set(0); }

noise::noise(double a) : a(a) {}
void noise::generate(unit & u) { FOR_SU(i) u.y[i] = rnd(-a, a); }

double pulse::start() { const double x = at(); ++k; return x; }
double pulse::at() { return (SU /(double) SR) * k; }
bool pulse::more() { return m->s > at(); }
pulse::pulse(mv_ptr m) : m(m), k() {}
void pulse::generate(unit & u)
{
    double x = start();
    const double d = 1./SR;
    FOR_SU(i) u.y[i] = m->z(x += d);
}

hung::hung(mv_ptr m) : pulse(m) {}
bool hung::more() { return true; }
void hung::generate(unit & u)
{
    if (at() < m->s) return pulse::generate(u);
    const double y = m->z(m->s);
    FOR_SU(i) u.y[i] = y;
}

bool record::more() { return c < b.w.size(); }
unsigned record::size() { return b.w.size(); }
void record::reset()
{
    t += c /(double) SR;
    b.cycle(duration->z(t) * SR);
    c = 0;
}

record::record(generator & g, mv_ptr duration)
    : b(duration->z(0) * SR), duration(duration), c(), t()
{
    fill(g, b.w.begin(), b.w.size());
}

void record::generate(unit & u)
{
    if (c + SU < b.w.size()) {
        FOR_SU(i) u.y[i] = b.w[c++];
    } else if (c < b.w.size()) {
        const unsigned h = b.w.size() - c;
        FOR(i, 0, h) u.y[i] = b.w[c++];
        FOR(i, h, SU) u.y[i] = 0;
    } else {
        u.set(0);
    }
}

periodic::buffer::buffer() : head(), tail(), a() {}
unsigned periodic::buffer::n()
{
    if (head == tail) return 0;
    else if (head > tail) return head - tail;
    else return head + (2*SU - tail);
}
unsigned periodic::buffer::post_incr(unsigned & i)
{
    const unsigned r = i;
    if (++i >= 2*SU) i = 0;
    return r;
}
void periodic::buffer::put(double y) { a[post_incr(head)] = y; }
double periodic::buffer::get() { return a[post_incr(tail)]; }

void periodic::append(unit & u, unsigned n)
{
   FOR(i, 0, n) carry->put(u.y[i]);
}

void periodic::shift(unit & u)
{
    if (carry->n() < SU) throw 1;
    FOR_SU(i) u.y[i] = carry->get();
}

periodic::periodic(pg_ptr && g) : g(std::move(g)), carry(new buffer) {}

void periodic::generate(unit & u)
{
    for (;;) {
        unsigned n = SU;
        unit v;
        g->generate(v);
        if ( ! g->more()) {
            if (unsigned r = g->size() % SU) n = r;
            g->reset();
        }
        append(v, n);
        if (carry->n() >= SU) break;
    }
    shift(u);
}

karpluss::karpluss(fl_ptr l, generator & g, mv_ptr duration)
    : record(g, duration), l(l)
{}

void karpluss::reset()
{
    record::reset();
    for (double & x : b.w) x = l->shift(x);
}

multiply::multiply(ug_ptr && a, ug_ptr && b)
    : a(std::move(a)), b(std::move(b))
{}

void multiply::generate(unit & u)
{
    unit v;
    a->generate(v);
    b->generate(u);
    FOR_SU(i) u.y[i] *= v.y[i];
}

bool multiply::more() { return a->more() && b->more(); }

product::product() : anymore() {}

void product::c(ug_ptr && g, double w)
{
    s.emplace_back(std::move(g), w);
    anymore = true;
}

void product::generate(unit & u)
{
    anymore = false;
    u.set(1);
    for (unsigned i=0; i!=s.size(); i++) {
        if (s[i].e->more()) {
            unit v;
            s[i].e->generate(v);
            u.mul(v, s[i].w);
            if (s[i].e->more()) anymore = true;
        }
    }
    prune(s);
}

bool product::more() { return anymore; }

sum::sum() : anymore() {}

void sum::c(ug_ptr && g, double w)
{
    s.emplace_back(std::move(g), w);
    anymore = true;
}

void sum::generate(unit & u)
{
    anymore = false;
    u.set(0);
    for (unsigned i=0; i!=s.size(); i++) {
        if (s[i].e->more()) {
            unit v;
            s[i].e->generate(v);
            u.add(v, s[i].w);
            if (s[i].e->more()) anymore = true;
        }
    }
    prune(s);
}

bool sum::more() { return anymore; }

modulation::modulation(ug_ptr && m, en_ptr c, mv_ptr f)
    : m(std::move(m)), c(c), f(f), x(), t()
{}

void modulation::generate(unit & u)
{
    unit v;
    m->generate(v);
    double s = t;
    t += SU /(double) SR;
    double d = 0;
    FOR_SU(i)
    {
        if (i % SC == 0) {
            d = f->z(s) / SR;
            s += SC /(double) SR;
        }
        u.y[i] = c->y(x);
        mod1inc(x, d * (1 + v.y[i]));
    }
}

delayed_sum::entry::entry(double t, ug_ptr && g)
    : t(t), g(std::move(g)), offset(unsigned(t * SR) % SU)
{}

delayed_sum::couple::couple() : c(2) { a.set(0); b.set(0); }
void delayed_sum::couple::add(unsigned h, unit & u)
{
    unsigned i = 0;
    FOR(j, h, SU) a.y[j] += u.y[i++];
    FOR(j, 0, h) b.y[j] += u.y[i++];
    c = 0;
}
void delayed_sum::couple::flush(unit & u)
{
    u = a;
    a = b;
    b.set(0);
    ++c;
}
bool delayed_sum::couple::carry() { return c < 2; }

delayed_sum::delayed_sum() : v(new couple), pending(), k() {}

void delayed_sum::c(ug_ptr && g, double t)
{
    if (!entries.empty() && entries.back().t > t) throw 1;
    entries.emplace_back(t, std::move(g));
    pending = true;
}

void delayed_sum::generate(unit & u)
{
    pending = false;
    const double s = ++k * SU /(double) SR;
    unsigned z = 0;
    bool h = false;
    for (auto & e : entries) {
        if (e.g->more()) {
            if (e.t >= s) {
                pending = true;
                break;
            }
            unit w;
            e.g->generate(w);
            v->add(e.offset, w);
            h = true;
        } else if ( ! h) ++z;
    }
    entries.erase(entries.begin(), entries.begin() + z);
    v->flush(u);
}

bool delayed_sum::more() { return pending || v->carry(); }

lazy::lazy(bs_ptr b) : b(b) {}

void lazy::generate(unit & u)
{
    g->generate(u);
}

bool lazy::more()
{
    if ( ! g) {
        g = b->build();
        b = nullptr;
    }
    return g->more();
}

void limiter::out(unit & u, double t)
{
    const double d = (t - g) / SU;
    double a = g;
    FOR_SU(j) {
        u.y[j] = v.y[j] * (a += d);
        if (std::fabs(u.y[j] > 1)) throw 1;
    }
}

limiter::limiter(ug_ptr && z) : z(std::move(z)), b(), q(), i(.01), s(1), g(1) {}

void limiter::generate(unit & u)
{
    if (!b) {
        b = true;
        z->generate(v);
    }
    unit w;
    z->generate(w);
    double a = 0;
    double b = 0;
    FOR_SU(j) {
        const double c = w.y[j];
        if (a < c) a = c;
        else if (c < b) b = c;
    }
    b = -b;
    if (a < b) a = b;
    const double m = (a > 1) ? 1/a : 1;
    double t = s;
    if (t > m) t = m;
    if (t > g + i) t = g + i;
    out(u, t);
    g = t;
    v = w;
    s = m;
}

bool limiter::more()
{
    if (q) return false;
    if (z->more()) return true;

    q = true;
    return true;
}

filtration::filtration(ug_ptr && g, fl_ptr l) : g(std::move(g)), l(l) {}

void filtration::generate(unit & u)
{
    g->generate(u);
    for (double & x : u.y) x = l->shift(x);
}

timed::timed(ug_ptr && g, double t) : g(std::move(g)), n(SR * t / SU), k() {}

void timed::generate(unit & u)
{
    g->generate(u);
    if (k <= n) ++k;

    if (k == n) {
        double a = 1;
        double d = a / SU;
        FOR_SU(i) {
            u.y[i] *= a;
            a -= d;
        }
    }
}

bool timed::more() { return k <= n; }
