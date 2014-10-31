#ifndef MATH_HPP
#define MATH_HPP

#include <cstdlib>
#include <cmath>

namespace {

double const pi = 3.1415926535;

double spow(double y, double p)
{ return (y >= 0) ? std::pow(y, p) : -std::pow(-y, p); }

double rnd(double a, double b)
{ return a + std::rand() * (b - a) / RAND_MAX; }

double linear(double a, double b, double r)
{ return a * (1 - r) + b * r; }

}//namespace

#endif
