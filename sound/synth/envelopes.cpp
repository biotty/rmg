//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "envelopes.hpp"

#include "generators.hpp"
#include "math.hpp"

envelope::~envelope() {}

double inverts::operator()(double x){ return 1 / x; }

scales::scales(double factor) : factor(factor) {}
double scales::operator()(double x) { return factor * x; }

stretched::stretched(en_ptr e_, double w)
{
    en_ptr b = P<extracted>(e_);
    e = P<warped>(b, scales(1 / w));
}

double stretched::y(double x) { return e->y(x); }

// idea: a brownian stair walker with a given average density of steps
//       -- used for "slow" stuff (instead of implicit
//       control_clock on filter controller only) by lib-user

extracted::extracted(en_ptr e) : y0(e->y(0)), y1(e->y(1)), e(e) {}
double extracted::y(double x)
{
    if (x <= 0) return y0;
    if (x >= 1) return y1;

    return e->y(x);
}

serial::serial(en_ptr a_, en_ptr b_, double h)
    : y0(a_->y(0))
    , y1(b_->y(1))
    , h(h)
    , a(P<stretched>(a_, h))
    , b(P<stretched>(b_, 1 - h))
{}

double serial::y(double x)
{
    if (x <= 0) return y0;
    if (x >= 1) return y1;

    if (x < h) return a->y(x);

    return b->y(x - h);
}

warped::warped(en_ptr e, fun f) : e(e), f(f) {}
double warped::y(double x) { return e->y(f(x)); }

constant::constant(double k) : k(k) {}
double constant::y(double) { return k; }

shaped::shaped(en_ptr e, fun f) : e(e), f(f) {}
double shaped::y(double x) { return f(e->y(x)); }

added::added(en_ptr a, en_ptr b) : a(a), b(b) {}
double added::y(double x) { return a->y(x) + b->y(x); }

en_ptr make_isosceles(double h, double y1, double t)
{
    const double d = h / t;  // note: h abs-time. while result stretched
    pe_ptr a = P<punctual>(0, 0);
    a->p(0 + d, y1);
    a->p(1 - d, y1);
    return P<stretched>(a, t);
}

en_ptr make_stroke(en_ptr e, double h, double t)
{
    return P<shaped>(
            P<stretched>(
                P<serial>(
                    e,
                    P<punctual>(e->y(1), 0), h / t),
                t),
            [](double x){ return x * x; });
}

functional::functional(fun f) : f(f) {}

double functional::y(double x) { return f(x); }

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
    if (x <= points.front().first) return points.front().second;
    if (x >= points.back().first) return points.back().second;
    const unsigned i = offset(x);
    // optimization: cache previous i.
    //   if i same, then we CAN calculate
    //   y = w * x + z FOR SOME w AND z that we could calculate
    //   INSTEAD OF doing the following, slightly more heavy.
    //   also, the offset function could start at hint previous i - 1
    //   that is checked to be below x inside of offset_hint(x, i)
    const double a = points[i - 1].first;
    const double b = points[i].first;
    const double t = (x - a) / (b - a);
    return linear(points[i - 1].second, points[i].second, t);
}

double tabular::y(double x)
{
    if (x <= 0) return values.front();
    if (x >= 1) return values.back();
    return values[x * values.size()];
}

sine::sine(double phase) : phase(phase) {}
double sine::y(double x) { return sin(2 * pi * (x + phase)); }
// optimization: use a tabular of a lib-init populated quarter of the period
//  this is not the interpolated tabular variant (have two variants)

