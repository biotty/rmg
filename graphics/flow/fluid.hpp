//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef FLUID_HPP
#define FLUID_HPP

#include "flow.hpp"
#include "planar.hpp"
#include "linear.hpp"


struct FluidCell
{
    XY velocity;  // north-west corner of area covered by cell
    float rotation;  // clockwise -- unit means each side moved by 1
    float pressure;  // actually mass, but cell-size is 1 per definition

    FluidCell() : rotation(0), pressure(1) {}
    operator XY() const { return velocity; }
    void operator=(const XY & v) { velocity = v; }
};


typedef Flow<FluidCell> Fluid;
typedef FlowFunction<FluidCell> FluidFunction;


double mass_at_nw(const Neighborhood<FluidCell> & q, double density)
{
    return (q.c.pressure + q.nw.pressure) * (0.5 * density);
}


double pressure_drop(XY a, XY b, double density)
{
    // drops from a to b
    return (b.abs() - a.abs()) * (density * 0.5);
}


XY venturi_force(const Neighborhood<FluidCell> & q, double density)
{
    double x = pressure_drop(q.w.velocity, q.e.velocity, density);
    double y = pressure_drop(q.n.velocity, q.s.velocity, density);
    return XY(x, y) * 0.5; // halve as distance is 2
}


XY pushed_velocity(const Neighborhood<FluidCell> & q,
        double t, double density)
{
    double nw = q.nw.pressure;
    double ne = q.n.pressure;
    double sw = q.w.pressure;
    double se = q.c.pressure;
    XY venturi = venturi_force(q, density);
    double x = ((nw + sw) - (ne + se)) * 0.5 + venturi.x;
    double y = ((nw + ne) - (sw + se)) * 0.5 + venturi.y;
    return q.c.velocity + XY(x, y) * (t / mass_at_nw(q, density));
}


double inflated_volume(const Neighborhood<FluidCell> & q, double t)
{
    double x = ((q.e.velocity.x + q.se.velocity.x)
            - (q.c.velocity.x + q.s.velocity.x)) * 0.5 * t;
    double y = ((q.s.velocity.y + q.se.velocity.y)
            - (q.c.velocity.y + q.e.velocity.y)) * 0.5 * t;
    return (1 + x) * (1 + y);
}


struct Movement
{
    const XY velocity;
    const double rotation;  // note: clockwise

    Movement(const XY & velocity, double rotation)
            : velocity(velocity), rotation(rotation) {}
    Movement(const FluidCell & c)
            : velocity(c.velocity), rotation(c.rotation) {}

    double n() const { return velocity.x + rotation; }
    double s() const { return velocity.x - rotation; }
    double w() const { return velocity.y - rotation; }
    double e() const { return velocity.y + rotation; }
};


XY average_velocity(const Neighborhood<FluidCell> & q)
{   // movement on the sides of the center-cell
    return XY(Movement(q.n).s() + Movement(q.s).n()
            + q.w.velocity.x + q.e.velocity.x,
        Movement(q.w).e() + Movement(q.e).w()
            + q.n.velocity.y + q.s.velocity.y) * 0.25;
}


double angular_momentum(const Neighborhood<FluidCell> & q)
{
    return (Movement(q.n).s() - Movement(q.s).n()
            - Movement(q.w).e() + Movement(q.e).w()) * 0.25;
}


Movement influenced_movement(const Neighborhood<FluidCell> & q, double t,
        double density, double viscosity)
{
    double factor = viscosity * t / mass_at_nw(q, density);
    double rotation = q.c.rotation + angular_momentum(q) * factor;
    XY velocity = linear(q.c.velocity, average_velocity(q), factor);
    return Movement(velocity, rotation);
}


#define DEFINE_ADVECTION(T, A, E) \
T advected_ ## A(const Neighborhood<FluidCell> & q, double t) \
{   XY d = E; \
    double x = std::fabs(d.x); \
    double y = std::fabs(d.y); \
    if (x > 1) { std::cerr << x << " > 1\n"; x = 1; } \
    if (y > 1) { std::cerr << y << " > 1\n"; y = 1; } \
    y *= 1 - x; /* let corner count as equatorial advection*/ \
    T a = linear<T>(q.c.A, d.y > 0 ? q.n.A : q.s.A, y); \
    return linear<T>(a, d.x > 0 ? q.w.A : q.e.A, x); \
}
DEFINE_ADVECTION(XY, velocity, q.c.velocity * t)
DEFINE_ADVECTION(double, rotation, (q.c.velocity + q.se.velocity) * (0.5 * t))
DEFINE_ADVECTION(double, pressure, (q.c.velocity + q.se.velocity) * (0.5 * t))


struct FluidParameters
{
    virtual ~FluidParameters() {}
    virtual void setup(double t, double u) = 0;
    virtual double density(const Position & position) = 0;
    virtual double viscosity(const Position & position) = 0;
};


enum class aspect_t {friction, volume, advection};
struct FluidAspectFunction : FluidFunction
{
    const aspect_t aspect;
    FluidParameters & params;

    FluidAspectFunction(aspect_t a, FluidParameters & p) : aspect(a), params(p) {}
    bool operator()(Grid<FluidCell> * field_swap, Grid<FluidCell> * field, double step_t)
    {
        for (PositionIterator it = field->positions(); it.more(); ++it) {
            FluidCell & b = field_swap->cell(it);
            const double density = params.density(it.position);
            const double viscosity = params.viscosity(it.position);
            Neighborhood<FluidCell> q = field->neighborhood(it);
            switch (aspect) {
            case aspect_t::friction:
                {   //advection (step 2) doesnt *correctly* handle |displacement| > 1
                    const double velocity_limit = /*some below 1*/0.7 / step_t;
                    double a = q.c.velocity.abs();
                    if (a > velocity_limit) {
                        std::cerr << "displacement close to 1 at [" << it.position.i <<
                            "][" << it.position.j << "] is " << q.c.velocity.x << ","
                            << q.c.velocity.y << "\n";
                        //commented (throw): because catcher doesnt help in any way
                        //throw a*step_t;//retry flow-iteration with smaller steps
                        //(won't swap, so results are discarded)
                        //note: in order to stabilize, i could actually throw
                        //      some stabilizer itself.  it could smooth all
                        //      velocities (could instantiate an aspect 3 doing that)
                    }
                }

                {
                    Movement m = influenced_movement(q, step_t, density, viscosity);
                    b.velocity = m.velocity;
                    b.rotation = m.rotation;
                }
                b.pressure = q.c.pressure;  // no molecular diffusion
            break;
            case aspect_t::volume:
                b.velocity = pushed_velocity(q, step_t, density);
                b.rotation = 0;
                b.pressure = q.c.pressure / inflated_volume(q, step_t);
            break;
            case aspect_t::advection:
                b.velocity = advected_velocity(q, step_t);
                b.rotation = advected_rotation(q, step_t);
                b.pressure = advected_pressure(q, step_t);
            break;
            }
        }
        return true;  // do swap
    }
};


struct FluidIndicator
{
    virtual void lapse(double delta_t, Fluid & flow, size_t i) = 0;
};


struct FluidAnimation
{
    ~FluidAnimation()
    {
        for (size_t aspect = 0; aspect < 3; ++aspect)
            delete functions[aspect];
    }

    FluidAnimation(size_t h, size_t w, FluidParameters & p, FluidFunction & f)
            : flow(h, w), params(p)
    {
        for (int a = 0; a < 3; ++a)
            functions.push_back(new FluidAspectFunction(
                        static_cast<aspect_t>(a), params));
        functions.push_back(&f);
    }
    
    void run(size_t n, FluidIndicator & indicator, double d = 1)
    {
        std::cout << n << ":\n";
        size_t s = 32;
        double u = d * n;
        for (size_t i = 0; i < n; ++i) {
            double t = d * i;
            params.setup(t, u);
            flow.lapse(d, s, functions);
            indicator.lapse(d, flow, i);
        }
        std::cout << "\n";
    }

private:
    Fluid flow;
    FluidParameters & params;
    std::vector<FluidFunction *> functions;
};


#endif

