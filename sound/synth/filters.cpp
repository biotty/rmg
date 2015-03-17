//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "filters.hpp"
#include "math.hpp"

period_buffer::period_buffer(unsigned n) : f(rnd(0, 1)), w(n) {}

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

delay_network::delay_network(fl_ptr l, double s)
        : i(), b(s * SR), l(l), s(s)
{}

void delay_network::step(double z)
{
    b.w[i] = z;
    if (++i == b.w.size()) {
        b.cycle(s * SR);
        for (auto & x: b.w) x = l->shift(x);
        i = 0;
    }
}

double delay_network::current() { return b.w[i]; }

control_clock::control_clock() : i(), x() {}

void control_clock::tick(std::function<void(double)> f)
{
    if (i == 0) {
        f(x);
        i = SC;
        x += SC /(double) SR;
    }
    --i;
}

feed::feed(fl_ptr l, mv_ptr f, en_ptr s) : d(l, s->y(0)), f(f), g(), s(s) {}

double feed::shift(double y)
{
    c.tick(
        [this](double x)
        {
            this->g = this->f->z(x);
            if (x < this->f->s) this->d.s = this->s->y(x / this->f->s);
        }
    );
    const double z = linear(y, d.current(), g);
    d.step(back ? z : y);
    return z;
}

feedback::feedback(fl_ptr l, mv_ptr f, en_ptr s) : feed(l, f, s)
{
    back = true;
}

biquad::biquad(biquad::control c)
    : fc(c), w1(), w2(), b0(), b1(), b2(), a1(), a2()
{}

double biquad::shift(double x)
{
    cc.tick(
        [this](double x)
        {
            const double s = this->fc.b0->s;
            if (x >= s) return;
            this->b0 = this->fc.b0->z(x);
            const double z = x / s;
            this->b1 = this->fc.b1->y(z);
            this->b2 = this->fc.b2->y(z);
            this->a1 = this->fc.a1->y(z);
            this->a2 = this->fc.a2->y(z);
        }
    );
    const double w0 = x - a1 * w1 - a2 * w2;
    const double y = b0 * w0 + b1 * w1 + b2 * w2;
    w2 = w1;
    w1 = w0;
    return y;
}
