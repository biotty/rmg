//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "envelopes.hpp"

#include "generators.hpp"
#include "math.hpp"

envelope::~envelope() {}

shaped::shaped(en_ptr e, fun f) : e(e), f(f) {}
double shaped::y(double x) { return f(e->y(x)); }

warped::warped(en_ptr e, fun f) : e(e), f(f) {}
double warped::y(double x) { return e->y(f(x)); }

constant::constant(double k) : k(k) {}
double constant::y(double) { return k; }

subtracted::subtracted(en_ptr a, en_ptr b) : a(a), b(b) {}
double subtracted::y(double x) { return a->y(x) - b->y(x); }

added::added(en_ptr a, en_ptr b) : a(a), b(b) {}
double added::y(double x) { return a->y(x) + b->y(x); }

scaled::scaled(en_ptr e, double m) : e(e), m(m) {}
double scaled::y(double x) { return m * e->y(x); }

inverted::inverted(en_ptr e) : e(e) {}
double inverted::y(double x) { return 1 / e->y(x); }

squared::squared(en_ptr e) : e(e) {}
double squared::y(double x) { const double y = e->y(x); return y * y; }

expounded::expounded(en_ptr e) : e(e) {}
double expounded::y(double x) { return exp(e->y(x)); }

movement::movement(en_ptr e, double s) : e(e), s(s) {}
double movement::z(const double t)
{
    if (t <= 0) return e->y(0);
    if (t >= s) return e->y(IB1);
    return e->y(t / s);
}

movement::~movement() {}

still::still(double k, double t) : movement(P<constant>(k), t) {}

stroke::stroke(en_ptr e, double h, double s) : movement(e, s), h(h) {}
double stroke::z(const double t) { const double y = a(t); return y * y; }
double stroke::a(const double t)
{
    if (t < 0 || t >= s) return 0;
    if (t < h) return e->y(t / h);
    else return linear(e->y(IB1), 0, (t - h) / (s - h));
}

punctual::punctual() : points({{0, 0}, {1, 0}}) {}

punctual::punctual(double y0, double y1) : points({{0, y0}, {1, y1}}) {}

unsigned punctual::offset(double x)
{
    unsigned i = 1;
    while (points.at(i).first < x) ++i;
    return i;
}

void punctual::p(double x, double y)
{
    points.insert(points.begin() + offset(x), {x, y});
}

double punctual::y(double x)
{
    if (x < 0 || x >= 1) return 0;
    if (x >= IB1) return points.back().second;
    const unsigned i = offset(x);
    // optimization: cache previous i.
    //   if i same, then we CAN calculate
    //   y = w * x + z FOR SOME w AND z that we could calculate
    //   INSTEAD OF doing the following, slightly more heavy.
    const double a = points[i - 1].first;
    const double b = points[i].first;
    const double t = (x - a) / (b - a);
    return linear(points[i - 1].second, points[i].second, t);
}

tabular::tabular(ug_ptr g, unsigned n)
{
    a.resize(n + SU);
    unsigned k = 0;
    while (g->more()) {
        unit u;
        g->generate(u);
        FOR_SU(i) a[i + k] = u.y[i];
        k += SU;
    }
    a.resize(n);
}

double tabular::y(double x)
{
    if (x < 0 || x >= 1) return 0;
    if (x >= IB1) return a.back();
    return a[x * a.size()];
}

#define PI 3.141592653589793

sine::sine(double phase) : phase(phase) {}
double sine::y(double x) { return sin(2 * PI * (x + phase)); }
