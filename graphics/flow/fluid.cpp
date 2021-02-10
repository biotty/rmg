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
    double esq;
    color c;

    ColorMatch(double e, color c) : esq(e * e), c(c) {}

    bool operator()(color m) {
        return similar(esq, &c, &m);
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
        int dc = 0;
        int vc = 0;
        for (size_t i = 0; i < palette.size(); ++i) {
            const color & c = palette[i];
            double d = density;
            double v = viscosity;
            for (size_t j = 0; j < density_cms.size(); ++j) {
                if (density_cms[j](c)) {
                    d = density_e;
                    dc++;
                    break;
                }
            }
            for (size_t k = 0; k < viscosity_cms.size(); ++k) {
                if (viscosity_cms[k](c)) {
                    v = viscosity_e;
                    vc++;
                    break;
                }
            }
            densities[i] = d;
            viscosities[i] = v;
        }
        std::cerr << "pal-exc-dens:" << dc << " pal-exc-visc:" << vc << std::endl;
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


struct JetFunction : FluidFunction
{
    Position pos;
    double a;
    double w;
    double r;

    JetFunction(Position pos, double a, double w, double r, FluidParameters & fp)
        : pos(pos), a(a), w(w), r(r), params(fp)
    {}
    bool operator()(Grid<FluidCell> * /*field_swap*/, Grid<FluidCell> * field, double step_t)
    {
        XY force = XY(cos(a), sin(a)) * r;
        a += w * step_t;
        field->cell(pos.i, pos.j).velocity += force * (step_t / params.density(pos));
        return false;
    }

    static JetFunction * parse(std::istream & ist, FluidParameters & fp)
    {
        size_t i, j;
        double a, w, r;
        ist >> i >> j >> a >> w >> r;
        if ( ! ist.good()) return nullptr;
        else return new JetFunction(Position(i, j), a, w, r, fp);
    }

private:
    FluidParameters & params;
};


struct ExtraFunction : FluidFunction
{
    Position pos;
    double e;

    ExtraFunction(Position pos, double e)
        : pos(pos), e(e)
    {}
    bool operator()(Grid<FluidCell> * /*field_swap*/, Grid<FluidCell> * field, double step_t)
    {
        field->cell(pos.i, pos.j).pressure *= step_t * e;
        return false;
    }
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
"-m N       output-image pixel-widths per fluid-cell\n"
"-M MODE    0:random 1:whirlpool\n"  // TODO: rather have a func on cell rotations
"-n N       count of output-images to produce\n"
"-o PATH    prefix for path to JPEG output-files\n"
"-q N       number of force-spots applied to fluid\n"
"-p N       number of extract/inject spots applied\n"
"-s N       seed for random-number-generator\n"
"-v REAL    normal and exceptional viscosity, respectively\n"
"-V REAL\n\n"
"-I         read 'i j a w r ..repeated for force-spots'\n"
"-R WxH     resolution, instead of as input\n";
}


int main(int argc, char **argv)
{
    int n = 512, M = 0;
    time_t seed = 0;
    unsigned p = 2, q = 9, m = 4;
    std::vector<ColorMatch> d_exc, v_exc;
    double z = 0.2, d = 0.08, D = 0.4, v = 0.4, V = 2.0;
    const char *image_prefix = "",
          *photo_filename = "img.png",
          *res_override = nullptr;
    bool is_to_read = false;

    int opt;
    while ((opt = getopt(argc, argv, "c:C:d:D:hi:m:M:n:o:p:q:s:v:V:R:I")) >= 0)
    switch (opt) {
        default: return 1;
        case 'c':
            {
                char e;
                unsigned r, g, b;
                if (std::sscanf(optarg, "%u,%u,%u/%c", &r, &g, &b, &e) != 4
                        || (e != 'd' && e != 'v')
                        || r > 255 || g > 255 || b > 255) {
                    std::cerr << "exception-color format is R,G,B/[dv]\n"
                        "example: -c 255,0,0/d -C .1 for dense red\n";
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
        case 'h': help(); return 0;
        case 'i': photo_filename = optarg; break;
        case 'm': m = std::atoi(optarg); break;
        case 'M': M = std::atoi(optarg); break;
        case 'n': n = std::atoi(optarg); break;
        case 'o': image_prefix = optarg; break;
        case 'p': p = std::atoi(optarg); break;
        case 'q': q = std::atoi(optarg); break;
        case 's': seed = std::atoi(optarg); break;
        case 'v': std::sscanf(optarg, "%lf", &v); break;
        case 'V': std::sscanf(optarg, "%lf", &V); break;
        case 'R': res_override = optarg; break;
        case 'I': is_to_read = true; break;
    }
    if (optind < argc) {
        std::cerr << "non-option argument\n";
        return 1;
    }
    if (seed == 0) {
        std::time(&seed);
    }

    PhotoColorizer c(photo_filename, image_prefix);
    auto dim = c.painting->dim();
    unsigned w_image = dim.first;
    unsigned h_image = dim.second;
    if (res_override) {
        if (std::sscanf(res_override, "%ux%u", &w_image, &h_image) != 2) {
            std::cerr << "resolution value takes format WxH\n";
            return 1;
        }
    }
    if (w_image % m || h_image % m) {
        std::cerr << "illegal dimentions "
            << w_image << "x" << h_image << " for -m " << m << "\n"
            "value must divide both width and height\n";
        return 1;
    }
    if (d_exc.empty()) {
        d_exc.push_back(ColorMatch(0.7, {0, 0, 0}));
        std::cerr << "using dark colors for except.density\n";
    }
    if (v_exc.empty()) {
        v_exc.push_back(ColorMatch(0.7, {1, 1, 1}));
        std::cerr << "using light colors for except.viscosity\n";
    }

    Tracer<palette_index_type> tracer(h_image, w_image);
    ColorTracer color_tracer(&tracer, &c);
    size_t h = h_image / m;
    size_t w = w_image / m;
    FeedbackParameters fp(h, w);
    std::vector<FluidFunction *> functions;
    std::srand(seed);
    std::vector<Position> pos;
    for (size_t k = 0; k < p + q; ++k) {
        size_t i = rnd(h);
        size_t j = rnd(w);
        pos.push_back(Position(i, j));
        // ^ pick prng sequence for just positions
    }
    if (M == 0) {
        for (unsigned k = 0; k < p; ++k) {
            functions.push_back(new ExtraFunction(pos[k], .98 + rnd(.04)));
        }
        for (unsigned k = p; k < p + q; ++k) {
            functions.push_back(new JetFunction(pos[k],
                        rnd(M_PI * 2), rnd(.01), .1 + rnd(.9), fp));
        }
    } else {
        XY center{ .5 * w, .5 * h };
        double inv_max_r = 2 / XY{ (double)w, (double)h }.abs();
        for (unsigned k = 0; k < p; ++k) {
            XY at{ (double)pos[k].j, (double)pos[k].i };
            double t = inv_max_r * (at - center).abs();
            functions.push_back(new ExtraFunction(pos[k], linear(.98, 1.02, t)));
        }
        for (unsigned k = p; k < p + q; ++k) {
            XY at{ (double)pos[k].j, (double)pos[k].i };
            XY v = at - center;
            double t = inv_max_r * v.abs();
            double a = atan2(v.y, v.x);
            functions.push_back(new JetFunction(pos[k], a + .5 * M_PI, 0, t, fp));
        }
    }
    pos.clear(); // nice: done with these
    if (is_to_read) {
        std::cerr << "read jets until eof\n";
        while (JetFunction * f = JetFunction::parse(std::cin, fp)) {
            if (f->pos.i >= h) std::cerr << "jet pos.i overflows h\n";
            if (f->pos.j >= w) std::cerr << "jet pos.j overflows w\n";

            functions.push_back(f);
        }
    }
    functions.push_back(new EdgeFunction<FluidCell>());
    CompositeFunction<FluidCell> cf(functions);
    FluidAnimation anim(h, w, fp, cf);

    fp.configure(color_tracer, d, v, d_exc, D, v_exc, V);
    anim.run(n, color_tracer);
    for (size_t k = 0; k < functions.size(); ++k) delete functions[k];
}
