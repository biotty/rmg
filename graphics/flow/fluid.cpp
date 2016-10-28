//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "fluid.hpp"
#include "tracer.hpp"
#include "palette.hpp"
#include <ctime>
#include <getopt.h>
#include <iostream>


struct ColorTracer : FluidIndicator
{
    Tracer<palette_index_type> * tracer;
    Colorizer * colorizer;

    ColorTracer(Tracer<palette_index_type> * tracer, Colorizer * colorizer)
            : tracer(tracer), colorizer(colorizer)
    {
        colorizer->initialize(tracer->trace);
    }

    void lapse(double delta_t, Fluid & flow, size_t i) {
        tracer->follow(flow.field, delta_t);
        colorizer->render_image(tracer->trace, i);
    }
};


struct ColorMatch
{
    double e;
    color c;

    ColorMatch(double e, color c) : e(e), c(c) {}

    bool operator()(color m) {
        return similar(e, &c, &m);
    }
};


struct FeedbackParameters : FluidParameters
{
    typedef std::map<palette_index_type, float> map_type;
    map_type densities;
    map_type viscosities;

    void configure(
            ColorTracer & color_tracer,
            double density, double viscosity,
            std::vector<ColorMatch> density_cms, double density_e,
            std::vector<ColorMatch> viscosity_cms, double viscosity_e)
    {
        const std::vector<color> & palette = color_tracer.colorizer->palette;
        tracer = color_tracer.tracer;
        for (size_t i = 0; i < palette.size(); ++i) {
            const color & c = palette[i];
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
    void setup(double /*t*/, double /*u*/)
    {
        if ( ! quantized) {
            quantized = new Grid<palette_index_type>(h, w);
        }
        quantize(*quantized, *tracer->trace);
    }

private:
    size_t h;
    size_t w;
    Tracer<palette_index_type> * tracer;
    Grid<palette_index_type> * quantized;
};


struct SimpleFunction : FluidFunction
{
    Position pos;
    double a;
    double w;
    double r;

    SimpleFunction(Position pos, double a, double w, double r, FluidParameters & fp)
        : pos(pos), a(a), w(w), r(r), params(fp)
    {}
    bool operator()(Grid<FluidCell> * /*field_swap*/, Grid<FluidCell> * field, double step_t)
    {
        XY force = XY(cos(a), sin(a)) * r;
        a += w * step_t;
        field->cell(pos.i, pos.j).velocity += force * (step_t / params.density(pos));
        return false;
    }

private:
    FluidParameters & params;
};


void help()
{
    std::cerr <<
"-c R,G,B/d add color to set for exceptional density or\n"
"   R,G,B/v viscosity.  R, G and B are 0 <= N < 256\n"
"-C REAL    color-closeness at following -c options\n"
"-d REAL    normal and exceptional density, respectively\n"
"-D REAL\n"
"-h         output this help to stdandard err\n"
"-i PATH    filename of PNG image to start with\n"
"-m N>0     output-image pixel-widths per fluid-cell\n"
"-n N>0     count of output-images to produce\n"
"-p PATH    prefix for path to JPEG output-files\n"
"-q N>0     number of force-spots applied to fluid\n"
"-s N>0     seed for random-number-generator\n"
"-v REAL    normal and exceptional viscosity, respectively\n"
"-V REAL\n"
"-x W,H     dimentions of the fluid grid\n";
}

// todo: smooth-out borders between colors.  advanced detection must
//       process and re-arrange when single-pixel peninsulas occure.


int main(int argc, char **argv)
{
    time_t seed = 0;
    unsigned q = 9, w = 256, h = 256, m = 3;
    std::vector<ColorMatch> d_exc, v_exc;
    double z = 0.1, d = 0.08, D = 0.2, v = 0.4, V = 1.0;
    const char *image_prefix = "", *photo_filename = "img.png";
    int n = 512;

    int opt;
    while ((opt = getopt(argc, argv, "c:C:d:D:hi:m:n:p:q:s:v:V:x:")) >= 0)
    switch (opt) {
        default:
            return 1;
        case 'c':
            {
                char e;
                unsigned r, g, b;
                if (std::sscanf(optarg, "%u,%u,%u/%c", &r, &g, &b, &e) != 4
                        || (e != 'd' && e != 'v')
                        || r > 255 || g > 255 || b > 255) {
                    std::cerr << "illegal format of exception-color\n"
                        "please give three X<256 folowed by '/d' or '/v'\n";
                    return 1;
                }
                std::vector<ColorMatch> & exc = (e == 'd') ? d_exc : v_exc;
                exc.push_back(ColorMatch(z,
                            {r /(real) 255, g /(real) 255, b /(real) 255}));
            }
            break;
        case 'C': std::sscanf(optarg, "%lf", &z); break;
        case 'd': std::sscanf(optarg, "%lf", &d); break;
        case 'D': std::sscanf(optarg, "%lf", &D); break;
        case 'h':
            help();
            return 0;
        case 'i':
            photo_filename = optarg;
            break;
        case 'm':
            if ((m = std::atoi(optarg)) <= 0) {
                std::cerr << "illegal optarg of -m\n"
                    "please give a positive number\n";
                return 1;
            }
            break;
        case 'n':
            if ((n = atoi(optarg)) <= 0) {
                std::cerr << "nothing to produce, as N is zero\n";
                return 1;
            }
            break;
        case 'p':
            image_prefix = optarg; break;
        case 'q':
            if ((q = std::atoi(optarg)) <= 0) {
                std::cerr << "q zero means to apply no forces to flow\n"
                    "the images would all be alike\n";
                return 1;
            }
            break;
        case 's':
            if ((seed = std::atoi(optarg)) <= 0) {
                std::cerr << "will use time as random-seed\n";
            }
            break;
        case 'v': std::sscanf(optarg, "%lf", &v); break;
        case 'V': std::sscanf(optarg, "%lf", &V); break;
        case 'x':
            if (std::sscanf(optarg, "%u,%u", &w, &h) != 2) {
                std::cerr << "illegal optarg of -x\n"
                    "please use two numbers separated by a ','\n";
                return 1;
            }
            break;
    }

    if (d_exc.empty()) {
        d_exc.push_back(ColorMatch(0.2, {0, 0, 0}));
        std::cerr << "using dark colors as exceptional density\n";
    }
    if (v_exc.empty()) {
        v_exc.push_back(ColorMatch(0.2, {1, 1, 1}));
        std::cerr << "using light colors as exceptional viscosity\n";
    }
    if (seed == 0) {
        std::time(&seed);
    }
    std::srand(seed);
    FeedbackParameters p(h, w);
    std::vector<FluidFunction *> functions;
    for (size_t k = 0; k < q; ++k) {
        Position pos(rnd(w), rnd(h));
        functions.push_back(new SimpleFunction(pos,
                    rnd(6.283), rnd(.01), .1 + rnd(.9), p));
    }
    functions.push_back(new EdgeFunction<FluidCell>());
    CompositeFunction<FluidCell> f(functions);
    FluidAnimation a(h, w, p, f);

    PhotoColorizer c(photo_filename, image_prefix);
    Tracer<palette_index_type> tracer(h * m, w * m);
    ColorTracer color_tracer(&tracer, &c);

    p.configure(color_tracer, d, v, d_exc, D, v_exc, V);
    a.run(n, color_tracer);
    for (size_t k = 0; k <=/*counting edge-function*/ q; ++k) delete functions[k];
}
