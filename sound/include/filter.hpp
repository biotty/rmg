//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef FILTER_HPP
#define FILTER_HPP

#include <vector>

namespace filter {

class delay
{
    std::vector<float> a;
    unsigned i;
public:
    delay();
    void alloc(unsigned n);
    double shift(double v);
};

class comb // in-operation adjustable combfilter
{
    std::vector<float> a;
    unsigned k, i;
public:
    comb(unsigned m);
    void setup();
    void adjust(unsigned m);
    double shift(double x);
};

class biquad
{
    double w1, w2;
public:
    double a1, a2, b0, b1, b2;
    biquad();
    double shift(double x);
    void lowpass(double s, double q = 0.7071);
    void highpass(double s, double q = 0.7071);
    void bandpass(double s, double q = 0.7071);
    void notch(double s, double q = 0.7071);
    void gain(double s, double q = 2);
    void lowshelf(double s, double a = 2, double z = 1/*upto one is monotonic*/);
    void highshelf(double s, double a = 2, double z = 1/*upto one is monotonic*/);
    void peaking_eq(double s, double a = 2, double b = 1/*octave*/);
};

}

#endif

