//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "fluid.hpp"
#include "tracer.hpp"
#include "palette.hpp"
#include <ctime>
#include <iostream>


struct ColorTracer : FluidIndicator
{
    Tracer<color_type> * tracer;
    Colorizer * colorizer;

    ColorTracer(Tracer<color_type> * tracer, Colorizer * colorizer)
            : tracer(tracer), colorizer(colorizer)
    {
        colorizer->initialize(tracer->trace);
    }

    void lapse(double delta_t, Fluid & flow, size_t i) {
        tracer->follow(flow.field, delta_t);
        colorizer->image(tracer->trace, i);
    }
};


struct ColorMatch
{
    double e;
    Color c;

    ColorMatch(double e, Color c) : e(e), c(c) {}

    bool operator()(Color m) {
        return similar(e, c, m);
    }
};


struct FeedbackParameters : FluidParameters
{
    typedef std::map<color_type, float> map_type;
    map_type densities;
    map_type viscosities;

    void configure(
            ColorTracer & color_tracer,
            double density, double viscosity,
            std::vector<ColorMatch> density_cms, double density_e,
            std::vector<ColorMatch> viscosity_cms, double viscosity_e)
    {
        const std::vector<Color> & palette = color_tracer.colorizer->palette;
        tracer = color_tracer.tracer;
        for (size_t i = 0; i < palette.size(); ++i) {
            const Color & c = palette[i];
            double d = density;
            double v = viscosity;
            for (size_t j = 0; j < density_cms.size(); ++j)
                if (density_cms[j](c)) {
                    d = density_e;
                    break;
                }
            for (size_t k = 0; k < viscosity_cms.size(); ++k)
                if (viscosity_cms[k](c)) {
                    v = viscosity_e;
                    break;
                }
            densities[i] = d;
            viscosities[i] = v;
        }
    }
    double density(const Position & p) { return densities.at(quantized->cell(p.i, p.j)); }
    double viscosity(const Position & p) { return viscosities.at(quantized->cell(p.i, p.j)); } 
    ~FeedbackParameters() { delete quantized; }
    FeedbackParameters(size_t h, size_t w) : h(h), w(w), tracer(NULL), quantized(NULL) {}
    void setup(double t, double u)
    {
        if ( ! quantized) {
            quantized = new Grid<color_type>(h, w);
        }
        quantize(*quantized, *tracer->trace);
    }

private:
    size_t h;
    size_t w;
    Tracer<color_type> * tracer;
    Grid<color_type> * quantized;
};


struct SimpleFunction : FluidFunction
{
    XY placement;
    XY force;

    SimpleFunction(XY p, XY f, FluidParameters & fp) : placement(p), force(f), params(fp) {}
    bool operator()(Grid<FluidCell> * field_swap, Grid<FluidCell> * field, double step_t)
    {
        size_t i = field->h * placement.y;
        size_t j = field->w * placement.x;
        field->cell(i, j).velocity += force * (step_t / params.density(Position(i, j)));
        return false;
    }

private:
    FluidParameters & params;
};


int main()
{
    std::srand(std::time(0));

    size_t q = 3;
    size_t h = 256;
    size_t w = 256;
    FeedbackParameters p(h, w);
    std::vector<FluidFunction *> functions;
    for (size_t k = 0; k < q; ++k) {
        XY placement(rnd(1), rnd(1));
        XY force(rnd(2) - 1, rnd(2) - 1);
        functions.push_back(new SimpleFunction(placement, force, p));
    }
    functions.push_back(new EdgeFunction<FluidCell>());
    CompositeFunction<FluidCell> f(functions);
    FluidAnimation a(h, w, p, f);

    PhotoColorizer c("img.png", "");
    Tracer<color_type> tracer(h * 3, w * 3);
    ColorTracer color_tracer(&tracer, &c);

    Color blue = {0, 0, 1};
    Color white = {1, 1, 1};
    p.configure(color_tracer,
        0.08, 0.4,
        { ColorMatch(0.1, blue) }, 1.0,
        { ColorMatch(0.1, white) }, 0.2);
    a.run(512, color_tracer);
    for (size_t k = 0; k <=/*counting edge-function*/ q; ++k) delete functions[k];
}

