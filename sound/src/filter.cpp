
#include "filter.hpp"
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

void biquad::lowpass(double s, double q)
{
    double w = 2 * pi * s / SAMPLERATE;
    double a0, alpha = .5 * sin(w) / q;
    b1 = 1 - cos(w);
    b0 = b1 * .5;
    b2 = b0;
    a1 = -2 * cos(w);
    a0 = 1 + alpha;
    a2 = 1 - alpha;
    multiply(1 / a0, *this);
}

void biquad::highpass(double s, double q)
{
    double w = 2 * pi * s / SAMPLERATE; // same as lowpass
    double a0, alpha = .5 * sin(w) / q;  // idem
    b1 = -1 - cos(w);
    b0 = b1 * -.5;
    b2 = b0;
    a1 = -2 * cos(w);  // same as lowpass
    a0 = 1 + alpha;  // ^
    a2 = 1 - alpha;  // ^
    multiply(1 / a0, *this);
}

void biquad::bandpass(double s, double q)
{
    double w = 2 * pi * s / SAMPLERATE; // same as lowpass
    double a0, alpha = .5 * sin(w) / q;  // idem
    b1 = 0;
    b0 = alpha;
    b2 = -b0;
    a1 = -2 * cos(w);  // same as lowpass
    a0 = 1 + alpha;  // ^
    a2 = 1 - alpha;  // ^
    multiply(1 / a0, *this);
}

void biquad::notch(double s, double q)
{
    double w = 2 * pi * s / SAMPLERATE; // same as lowpass
    double a0, alpha = .5 * sin(w) / q;  // idem
    b1 = -2 * cos(w);
    b0 = 1;
    b2 = 1;
    a1 = -2 * cos(w);  // same as lowpass
    a0 = 1 + alpha;  // ^
    a2 = 1 - alpha;  // ^
    multiply(1 / a0, *this);
}

void biquad::gain(double s, double q/*like a (in functions below) is peak gain*/)
{
    assert(q > 1); // maybe better to get param in in decibel
    double w = 2 * pi * s / SAMPLERATE; // same as lowpass
    double a0, alpha = .5 * sin(w) / q;  // idem
    b1 = 0;
    b0 = .5 * sin(w); // (== q * alpha)
    b2 = -b0;
    a1 = -2 * cos(w);  // same as lowpass
    a0 = 1 + alpha;  // ^
    a2 = 1 - alpha;  // ^
    multiply(1 / a0, *this);
}

void biquad::lowshelf(double s, double a, double z) // b in octaves, a is amplitude, s=1 is steepest is can be and still monotonicaly
{
    assert(a > 1); // maybe better to get param in in decibel
    double e = sqrt(a);
    double w = 2 * pi * s / SAMPLERATE; // same as lowpass
    double a0, alpha = .5 * sin(w) * sqrt((e + 1/e)*(1/z - 1) + 2); //NOT same (but could do q-based)
    b1 = 2 * e * ((e - 1) - (e + 1) * cos(w));
    b0 =     e * ((e + 1) - (e - 1) * cos(w) + 2 * sqrt(e) * alpha);
    b2 =     e * ((e + 1) - (e - 1) * cos(w) - 2 * sqrt(e) * alpha);
    a1 =    -2 * ((e - 1) + (e + 1) * cos(w));
    a0 =          (e + 1) + (e - 1) * cos(w) + 2 * sqrt(e) * alpha;
    a2 =          (e + 1) + (e - 1) * cos(w) - 2 * sqrt(e) * alpha;
    multiply(1 / a0, *this);
}

void biquad::highshelf(double s, double a, double z)
{
    assert(a > 1); // maybe better to get param in in decibel
    double e = sqrt(a);
    double w = 2 * pi * s / SAMPLERATE; // same as lowpass
    double a0, alpha = .5 * sin(w) * sqrt((e + 1/e)*(1/z - 1) + 2); //same as (but could do q-based)
    b1 =-2 * e * ((e - 1) + (e + 1) * cos(w));
    b0 =     e * ((e + 1) + (e - 1) * cos(w) + 2 * sqrt(e) * alpha);
    b2 =     e * ((e + 1) + (e - 1) * cos(w) - 2 * sqrt(e) * alpha);
    a1 =     2 * ((e - 1) - (e + 1) * cos(w));
    a0 =          (e + 1) - (e - 1) * cos(w) + 2 * sqrt(e) * alpha;
    a2 =          (e + 1) - (e - 1) * cos(w) - 2 * sqrt(e) * alpha;
    multiply(1 / a0, *this);
}

void biquad::peaking_eq(double s, double a, double b)// b in octaves, a is amplitude (factor <= 1.0)
{
    double e = sqrt(a);
    double w = 2 * pi * s / SAMPLERATE; // same as lowpass
    double a0, alpha = sin(w) * sinh(.5 * log(2) * b * w / sin(w));  // ! NOT same (but can instead do same q-based if desired)
    b1 = -2 * cos(w);
    b0 = 1 + alpha * e;
    b2 = 1 - alpha * e;
    a1 = -2 * cos(w);  // same as lowpass
    a0 = 1 + alpha / e;  // NOT same
    a2 = 1 - alpha / e;  // NOT same
    multiply(1 / a0, *this);
}

}

