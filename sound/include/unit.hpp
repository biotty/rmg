#ifndef UNIT_HPP
#define UNIT_HPP

#include <memory>

#define SU 441

#define FOR(X, A, B) for (unsigned X=A; X!=B; ++X)
#define FOR_SU(X) FOR(X, 0, SU)

struct unit
{
    double y[SU];

    void set(double s);
    void add(unit & u, double w);
    void mul(unit & u, double w);
};

struct generator
{
    virtual void generate(unit & u) = 0;
    virtual bool more() = 0;
};

typedef std::shared_ptr<generator> ug_ptr;

#endif

