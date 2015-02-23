//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef ENVELOPES_HPP
#define ENVELOPES_HPP

#include "unitfwd.hpp"
#include <vector>
#include <memory>
#include <functional>

struct envelope;
typedef std::shared_ptr<envelope> en_ptr;

struct envelope
{
    virtual double y(double x) = 0;
    virtual ~envelope();
};

typedef std::function<double (double)> fun;

struct shaped : envelope
{
    en_ptr e;
    fun f;

    shaped(en_ptr e, fun f);
    double y(double x);
};

struct warped : envelope
{
    en_ptr e;
    fun f;

    warped(en_ptr e, fun f);
    double y(double x);
};

struct constant : envelope
{
    double k;

    constant(double k);
    double y(double x);
};

struct subtracted : envelope
{
    en_ptr a;
    en_ptr b;

    subtracted(en_ptr a, en_ptr b);
    double y(double x);
};

struct added : envelope
{
    en_ptr a;
    en_ptr b;

    added(en_ptr a, en_ptr b);
    double y(double x);
};

struct scaled : envelope
{
    en_ptr e;
    double m;

    scaled(en_ptr e, double m);
    double y(double x);
};

struct inverted : envelope
{
    en_ptr e;

    inverted(en_ptr e);
    double y(double x);
};

struct squared : envelope
{
    en_ptr e;

    squared(en_ptr e);
    double y(double x);
};

struct expounded : envelope
{
    en_ptr e;

    expounded(en_ptr e);
    double y(double x);
};

#define IB1 1-1e-9

struct movement
{
    en_ptr e;
    double s;

    movement(en_ptr e, double s);
    virtual double z(double t);
    virtual ~movement();
};

typedef std::shared_ptr<movement> mv_ptr;

struct still : movement { still(double k, double t = 1); };

struct stroke : movement
{
    double h;

    stroke(en_ptr e, double h, double s);
    double a(double t);
    double z(double t);
};

struct punctual : envelope
{
    std::vector<std::pair<double, double>> points;
    unsigned offset(double x);

    punctual();
    punctual(double y0, double y1);
    double y(double x);
    void p(double x, double y);
};

typedef std::shared_ptr<punctual> pe_ptr;

struct tabular : envelope
{
    std::vector<double> a;

    tabular(ug_ptr g, unsigned n);
    double y(double x);
};

struct sine : envelope
{
    double phase;

    sine(double phase);
    double y(double x);
};

#endif
