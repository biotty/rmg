#ifndef WAVE_HPP
#define WAVE_HPP

#include "flow.hpp"
#include "planar.hpp"


struct WaveCell {
    XY velocity;
    XY displacement;
    
    operator XY() { return velocity; }
    void operator=(const XY & v) { velocity = v; }
};


typedef Flow<WaveCell> WaveFlow;
typedef FlowFunction<WaveCell> WaveFlowFunction;


XY drag_force(const SideNeighborhood<WaveCell> & q)
{
    return (q.w.displacement + q.e.displacement
          + q.n.displacement + q.s.displacement) * 0.25
        - q.c.displacement;
}


struct WaveFlowParameters
{
    virtual ~WaveFlowParameters() {}
    virtual void setup(double t, double u) = 0;
    virtual double density_inv(const Position & position) = 0;
};


struct WaveFunction : WaveFlowFunction
{
    WaveFlowParameters & params;

    WaveFunction(WaveFlowParameters & p) : params(p) {}
    bool operator()(Grid<WaveCell> * field_swap, Grid<WaveCell> * field, double step_t)
    {
        for (PositionIterator it = field->positions(); it.more(); ++it) {
            WaveCell & b = field_swap->cell(it);
            SideNeighborhood<WaveCell> q = field->side_neighborhood(it);
            b.displacement += q.c.velocity * step_t;
            b.velocity += drag_force(q) * step_t * params.density_inv(it.position);
        }
        return true;  // do swap
    }
};


struct WaveIndicator
{
    virtual void lapse(double delta_t, Grid<WaveCell> * grid, size_t i) = 0;
};


struct WaveFlowAnimation
{
    ~WaveFlowAnimation()
    {
        delete functions[0];
    }

    WaveFlowAnimation(size_t w, size_t h, WaveFlowParameters & p, WaveFlowFunction & f)
            : flow(h, w), params(p)
    {
        functions.push_back(new WaveFunction(params));
        functions.push_back(&f);
    }
    
    void run(size_t n, double d, size_t s, WaveIndicator & indicator)
    {
        std::cout << n << ":\n";
        double u = d * n;
        for (size_t i = 0; i < n; ++i) {
            double t = d * i;
            params.setup(t, u);
            flow.lapse(d, s, functions);
            indicator.lapse(d, flow.field, i);
        }
        std::cout << "\n";
    }

private:
    WaveFlow flow;
    WaveFlowParameters & params;
    std::vector<WaveFlowFunction *> functions;
};


#endif

