//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef ENVELOPES_HPP
#define ENVELOPES_HPP

#include "unitfwd.hpp"
#include <vector>
#include <memory>
#include <functional>

struct envelope : std::enable_shared_from_this<envelope>
{
    virtual double y(double x) = 0;
    virtual ~envelope();

    // re-design: have .stretch(w)=0 that yields as todays P<stretched>
    //            for functional (a warped) but manipulates (copied!) data
    //            on punctal and width (somehow keeps shared-ptr
    //            to the table) in tabular.  then remove stretched and
    //            use .stretch at callers instead.  resulting envelopes
    //            are to be efficient on operation with ::y (in incr x)
    //            recap: punctual have .stretch implemented, scaling on x-axis
    //            and tabular will be variable-width and .stretch is just
    //            making a new width "interface" to underlying table.
};

typedef std::shared_ptr<envelope> en_ptr;

struct stretched : envelope
{
    en_ptr e;

    stretched(en_ptr e_, double w);
    double y(double x);
};

struct extracted : envelope
{
    double y0;
    double y1;
    en_ptr e;

    extracted(en_ptr e);
    double y(double x);
};

struct serial : envelope
{
    double y0;
    double y1;
    double h;
    en_ptr a;
    en_ptr b;

    serial(en_ptr a_, en_ptr b_, double h);
    double y(double x);
};

struct inverts { double operator()(double x); };
struct scales
{
    double factor;
    scales(double factor);
    double operator()(double x);
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

struct added : envelope
{
    en_ptr a;
    en_ptr b;

    added(en_ptr a, en_ptr b);
    double y(double x);
};

en_ptr make_isosceles(double h, double y1, double t);
en_ptr make_stroke(en_ptr e, double h, double w);

struct functional : envelope
{
    fun f;

    functional(fun f);
    double y(double x);
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
    std::vector<double> values;

    double y(double x);
};

struct sine : envelope
{
    double phase;

    sine(double phase);
    double y(double x);
};

#endif
