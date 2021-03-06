//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "wave.hpp"
#include "linear.hpp"
#include "image.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <ctime>
#include <getopt.h>


constexpr double exaggerate = 5;


struct SqueezeIndicator : WaveIndicator
{
    const Picture & picture;
    size_t h;
    size_t w;
    std::string prefix;

    SqueezeIndicator(Picture & p, size_t h, size_t w, std::string prefix = "")
        : picture(p), h(h), w(w), prefix(prefix) {}

    void lapse(double /*delta_t*/, Grid<WaveCell> * grid, size_t i) {
        Grid<double> squeezes(grid->h, grid->w);
        for (PositionIterator it = grid->positions(); it.more(); ++it) {
            SideNeighborhood<WaveCell> q = grid->side_neighborhood(it);
            double pr = squeeze(q);
            squeezes.cell(it) = pr;
            bounds.update(pr);
        }
        const double s_m = bounds.factor() * exaggerate;
        std::ostringstream oss;
        oss << prefix << i << ".jpeg";
        image out = image_create(oss.str().c_str(), w, h);
        double h_m = squeezes.h /(double) h;
        double w_m = squeezes.w /(double) w;
        for (PositionIterator it(h, w); it.more(); ++it) {
            const Position & p = it.position;
            const double sq = squeezes.cell(p.i * h_m, p.j * w_m);
            color c = picture.color_at(XY(p.j/(double)it.w, p.i/(double)it.h));
            image_write(out, shaded_color(c, sq * s_m));
        }
        image_close(out);
        std::cout << i << "\r" << std::flush;
    }

private:
    double squeeze(const SideNeighborhood<WaveCell> & q)
    {
        return (q.w.displacement.x - q.e.displacement.x)
            + (q.n.displacement.y - q.s.displacement.y);
    }

    color shaded_color(color c, double sq)
    {
        real v = 1;
        real u = sq;
        if (u < 0) {
            v = 0;
            u *= -1;
        }
        if (u > 1) u = 1;
        c.r = linear(c.r, v, u);
        c.g = linear(c.g, v, u);
        c.b = linear(c.b, v, u);
        return c;
    }

    Bounds bounds;
};


struct ShakeFunction : WaveFlowFunction
{
    Position pos;
    double amplitude;

    ShakeFunction(Position pos, double a, double f, double af, double tz, double ts)
            : pos(pos)
            , amplitude(a)
            , frequency(f)
            , vaampfreq(af)
            , tilt_start(tz)
            , tilt_speed(ts), t(0)
    {}
    bool operator()(Grid<WaveCell> * /*field_swap*/, Grid<WaveCell> * field, double step_t)
    {
        double vaamp = -cos(t * vaampfreq);
        if (vaamp > 0) {
            double r = vaamp * amplitude * cos(t * frequency);
            double w = tilt_start + t * tilt_speed;
            field->cell(pos.i, pos.j).velocity = XY(cos(w), sin(w)) * r;
        }
        t += step_t;
        return false;
    }
    static ShakeFunction * parse(std::istream & ist)
    {
        size_t i, j;
        double a, f, af, tz, ts;
        ist >> i >> j >> a >> f >> af >> tz >> ts;
        if ( ! ist.good()) return nullptr;
        else return new ShakeFunction(Position(i, j), a, f, af, tz, ts);
    }

private:
    double frequency;
    double vaampfreq;
    double tilt_start;
    double tilt_speed;
    double t;
};


struct PictureParameters : WaveFlowParameters
{
    Picture & picture;
    size_t h;
    size_t w;

    PictureParameters(Picture & p, size_t h, size_t w) : picture(p), h(h), w(w) {}
    void setup(double /*t*/, double /*u*/)
    {
        picture.flip();
    }
    double density_inv(const Position & p)
    {
        XY u(p.j/(double)w, p.i/(double)h);
        return intensity(picture.color_at(u)) * 1.9 + 0.1;
    }
};


void help()
{
    std::cerr <<
"-h         output this help to stdandard err\n"
"-i PATH    filename of PNG image to start with\n"
"-m N>0     fluid-cell side in pixels\n"
"-n N>0     count of output-images to produce\n"
"-o PATH    prefix for path to JPEG output-files\n"
"-q N>0     number of force-spots applied to surface\n"
"-s N>0     seed for random-number-generator\n\n"
"-I         read 'i j a f af tz ts i j ...'\n"
"-R W,H     resolution override\n";
}


int main(int argc, char **argv)
{
    time_t seed = 0;
    const char *image_prefix = "",
          *photo_filename = "image.jpeg",
          *res_override = nullptr;
    unsigned m = 2, q = 10, n = 0;
    bool is_to_read = false;

    int opt;
    while ((opt = getopt(argc, argv, "hi:m:n:o:q:s:R:I")) >= 0)
    switch (opt) {
        default: return 1;
        case 'i': photo_filename = optarg; break;
        case 'm':
            if ((m = std::atoi(optarg)) <= 0) {
                std::cerr << "tile side must be a positive number\n";
                return 1;
            }
            break;
        case 'n': n = std::atoi(optarg); break;
        case 'o': image_prefix = optarg; break;
        case 'q': q = std::atoi(optarg); break;
        case 's': seed = std::atoi(optarg); break;
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

    Picture pic(photo_filename, n);
    if ( ! pic) return 1;
    unsigned w_image;
    unsigned h_image;
    pic.dim(w_image, h_image);
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
    size_t h = h_image / m;
    size_t w = w_image / m;
    PictureParameters p(pic, h, w);
    std::vector<WaveFlowFunction *> functions;
    std::srand(seed);
    std::vector<Position> pos;
    for (size_t k = 0; k < q; ++k) {
        size_t i = rnd(h);
        size_t j = rnd(w);
        pos.push_back(Position(i, j));
        // ^ pick prng sequence for just positions
    }
    for (size_t k = 0; k < q; ++k) {
        double a = rnd(1) + 0.5;
        double f = 1 / (rnd(7) + 1);
        double af = .01 / (rnd(6) + 1);
        double tz = rnd(3.14);
        double ts = rnd(0.2) - 0.1;
        functions.push_back(new ShakeFunction(pos[k], a, f, af, tz, ts));
    }
    pos.clear(); // nice: done with these
    if (is_to_read) {
        std::cerr << "read shakers until eof\n";
        while (ShakeFunction * f = ShakeFunction::parse(std::cin)) {
            if (f->pos.i >= h) std::cerr << "shaker pos.i overflows h\n";
            if (f->pos.j >= w) std::cerr << "shaker pos.j overflows w\n";

            functions.push_back(f);
        }
    }
    functions.push_back(new EdgeFunction<WaveCell>());
    CompositeFunction<WaveCell> f(functions);
    WaveFlowAnimation a(h, w, p, f);
    SqueezeIndicator indicator(pic, h_image, w_image, image_prefix);
    a.run(n, 2, 8, indicator);
    for (size_t k = 0; k < functions.size(); ++k) delete functions[k];
}
