//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef MATH_HPP
#define MATH_HPP

#include <cstdlib>
#include <cmath>

#define pi (3.1415926535)
#define spow(y, p) ((y >= 0) ? std::pow(y, p) : -std::pow(-y, p))
#define rnd(a, b) (a + std::rand() * (b - a) / RAND_MAX)
#define linear(a, b, r) (a * (1 - r) + b * r)

#endif
