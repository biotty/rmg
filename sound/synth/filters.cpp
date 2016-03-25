//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "filters.hpp"
#include "math.hpp"

period_buffer::period_buffer(unsigned n) : f(rnd(0, 1)), w(n) {}

// stretches buffer so that doppler effect result by use in feed(back)
// note that the use to obtain vibrato in a wave generator is
// discouraged because of degraded quality for an exact desired waveform
void period_buffer::cycle(double s)
{
    if (f > 1) {
        f -= 1;
        w.pop_back();
    }
    unsigned n = s;
    while (w.size() != n) {
        const unsigned i = rnd(1, w.size());
        const double v = .5 * (w[i - 1] + w[i]);
        if (w.size() < n) {
            w.insert(w.begin() + i, v);
        } else {
            w.erase(w.begin() + i);
            w[i - 1] = v;
        }
    }
    f += s - n;
    if (f > 1) w.push_back(.5 * (w.front() + w.back()));
}

double as_is::shift(double y) { return y; }

double strong::shift(double y)
{
    const double r = linear(y, w, a);
    w = y;
    return (b > 0 && b > rnd(0, 1)) ? -r : r;
}

strong::strong(double a, double b) : a(a), b(b), w() {}

delay_network::delay_network(en_ptr s, fl_ptr l)
        : i(), b(s->y(0) * SR), s(s), l(l), x_cycle()
{}

void delay_network::step(double z)
{
    b.w[i] = z;
    if (++i == b.w.size()) {
        x_cycle += i / (double) SR;
        b.cycle(s->y(x_cycle) * SR);
        if (l) for (auto & y: b.w) y = l->shift(y);
        // improvement: use the aproach of unset pointer as here
        //              instead of as_is filter (used elsewhere)
        //              so, the improvement is not here, which
        //              is already using unset pointer semantics
        i = 0;
    }
}

double delay_network::current() { return b.w[i]; }

control_clock::control_clock(unsigned a, unsigned b)
    : a(a), b(b), i(), x()
{}

bool control_clock::tick()
{
    if (i == 0) {
        i = a;
        if (b) i += rnd(0, b);
        x += i /(double) SR;
        return true;
    } else {
        --i;
        return false;
    }
}

feed::feed(en_ptr amount, en_ptr delay, bool back)
    : back(back), c(21), d(delay, fl_ptr()), amount(amount), g()
{}

double feed::shift(double y)
{
    if (c.tick()) g = amount->y(c.x);
    const double z = linear(y, d.current(), g);
    d.step(back? z : y);
    return z;
}

feedback::feedback(en_ptr amount, en_ptr delay)
    : feed(amount, delay, true)
{}

biquad::biquad(biquad::control c)
    : w1(), w2(), b0(), b1(), b2(), a1(), a2(), cc(11, 11), fc(c)
{}

double biquad::shift(double z)
{
    if (cc.tick()) {
        const double &x = cc.x;
        b0 = fc.b0->y(x);
        b1 = fc.b1->y(x);
        b2 = fc.b2->y(x);
        a1 = fc.a1->y(x);
        a2 = fc.a2->y(x);
    }
    const double w0 = z - a1 * w1 - a2 * w2;
    const double y = b0 * w0 + b1 * w1 + b2 * w2;
    w2 = w1;
    w1 = w0;
    return y;
}
