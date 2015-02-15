//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "biquad.hpp"
#include "samplerate.hpp"

#include <cstdlib>
#include <cmath>
#include <cassert>

namespace {

double const pi = 3.1415926535;
void multiply(double m, filter::biquad & f) {
    f.a1 *= m;
    f.a2 *= m;
    f.b0 *= m;
    f.b1 *= m;
    f.b2 *= m;
}

double rnd(double a, double b)
{ return a + std::rand() * (b - a) / RAND_MAX; }

double omega(double s) { return 2 * pi * s / SAMPLERATE; }
double alpha_pass(double w, double q) { return .5 * sin(w) / q; }
double alpha_shelf(double w, double e, double z)
{
    return .5 * sin(w) * sqrt((e + 1 / e) * (1 / z - 1) + 2);
}
double alpha_peaking(double w, double b)
{
    return sin(w) * sinh(.5 * log(2) * b * w / sin(w));
}

} //namespace

namespace filter {

delay::delay() : i() {}

void delay::alloc(unsigned n) { a.resize(n); }

double delay::shift(double v)
{
    if (a.size() == 0) return v;
    const double r = a[i];
    a[i++] = v;
    if (i == a.size()) i = 0;
    return r;
}

comb::comb(unsigned m) : a(), k(m), i() {}

void comb::setup() { a.resize(k); k = 0; }

void comb::adjust(unsigned m)
{
    unsigned n = a.size();
    if (m == n) return;
    if (m < n) {
        for (unsigned k=m; k<n; k++) {
            unsigned i = rnd(0, a.size());
            assert(i < a.size());
            a.erase(a.begin() + i);
        }
        return;
    }
    for (unsigned k=n; k<m; k++) {
        unsigned i = rnd(1, a.size());
        assert(i);
        a.insert(a.begin() + i, (a[i - 1] + a[i]) * .5);
    }
}

double comb::shift(double x)
{
    assert(k == 0);
    if (++i >= a.size()) i = 0;
    double y = (x + a[i]) * .5;
    a[i] = x;
    return y;
}

biquad::biquad() : w1(), w2(), a1(), a2(), b0(), b1(), b2() {}

double biquad::shift(double x)
{
    const double w0 = x - a1 * w1 - a2 * w2;
    const double y = b0 * w0 + b1 * w1 + b2 * w2;
    w2 = w1;
    w1 = w0;
    return y;
}

double biquad::set_a_pass(double w, double alpha)
{
    double a0;
    a1 = -2 * cos(w);
    a0 = 1 + alpha;
    a2 = 1 - alpha;
    return a0;
}

void biquad::lowpass(double s, double q)
{
    double w = omega(s);
    double a = alpha_pass(w, q);
    b1 = 1 - cos(w);
    b0 = b1 * .5;
    b2 = b0;
    multiply(1 / set_a_pass(w, a), *this);
}

void biquad::highpass(double s, double q)
{
    double w = omega(s);
    double a = alpha_pass(w, q);
    b1 = -1 - cos(w);
    b0 = b1 * -.5;
    b2 = b0;
    multiply(1 / set_a_pass(w, a), *this);
}

void biquad::bandpass(double s, double q)
{
    double w = omega(s);
    double a = alpha_pass(w, q);
    b1 = 0;
    b0 = a;
    b2 = -b0;
    multiply(1 / set_a_pass(w, a), *this);
}

void biquad::notch(double s, double q)
{
    double w = omega(s);
    double a = alpha_pass(w, q);
    b1 = -2 * cos(w);
    b0 = 1;
    b2 = 1;
    multiply(1 / set_a_pass(w, a), *this);
}

void biquad::gain(double s, double q)
{
    assert(q > 1);
    double w = omega(s);
    double a = alpha_pass(w, q);
    b1 = 0;
    b0 = .5 * sin(w);
    b2 = -b0;
    multiply(1 / set_a_pass(w, a), *this);
}

// z in octaves, a is ampl, s=1 is steepest when still monotonical
void biquad::lowshelf(double s, double a, double z)
{
    assert(a > 1);
    double e = sqrt(a);
    double d = e - 1;
    double u = e + 1;
    double w = omega(s);
    double c = cos(w);
    double k = alpha_shelf(w, e, z) * 2 * sqrt(e);
    b1 = 2 * e * (d - u * c);
    b0 =     e * (u - d * c + k);
    b2 =     e * (u - d * c - k);
    a1 =    -2 * (d + u * c);
    double a0 =   u + d * c + k;
    a2 =          u + d * c - k;
    multiply(1 / a0, *this);
}

// z in octaves, a is ampl, s=1 is steepest when still monotonical
void biquad::highshelf(double s, double a, double z)
{
    assert(a > 1);
    double e = sqrt(a);
    double d = e - 1;
    double u = e + 1;
    double w = omega(s);
    double c = cos(w);
    double k = alpha_shelf(w, e, z) * 2 * sqrt(e);
    b1 =-2 * e * (d + u * c);
    b0 =     e * (u + d * c + k);
    b2 =     e * (u + d * c - k);
    a1 =     2 * (d - u * c);
    double a0 =   u - d * c + k;
    a2 =          u - d * c - k;
    multiply(1 / a0, *this);
}

// b in octaves, a is amplitude (factor <= 1.0)
void biquad::peaking_eq(double s, double a, double b)
{
    double e = sqrt(a);
    double w = omega(s);
    double a0, p = alpha_peaking(w, b);
    b1 = -2 * cos(w);
    b0 = 1 + p * e;
    b2 = 1 - p * e;
    a1 = b1;
    a0 = 1 + p / e;
    a2 = 1 - p / e;
    multiply(1 / a0, *this);
}

}
