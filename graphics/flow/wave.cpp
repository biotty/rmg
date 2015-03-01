//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "wave.hpp"
#include "linear.hpp"
#include "image.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <ctime>


struct SqueezeIndicator : WaveIndicator
{
    const Picture & picture;
    size_t h;
    size_t w;
    std::string prefix;

    SqueezeIndicator(Picture & p, size_t h, size_t w) : picture(p), h(h), w(w), prefix("") {}
    void lapse(double /*delta_t*/, Grid<WaveCell> * grid, size_t i) {
        Grid<double> squeezes(grid->h, grid->w);
        for (PositionIterator it = grid->positions(); it.more(); ++it) {
            SideNeighborhood<WaveCell> q = grid->side_neighborhood(it);
            double pr = squeeze(q);
            squeezes.cell(it) = pr;
            bounds.update(pr);
        }
        const double s_m = bounds.factor() * 7;  // exaggerate
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
        double v = 1;
        double u = sq;
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


struct SimpleFunction : WaveFlowFunction
{
    XY placement;
    double amplitude;

    SimpleFunction(XY p, double a, double f, double tz, double ts)
            : placement(p)
            , amplitude(a)
            , frequency(f)
            , tilt_zero(tz)
            , tilt_speed(ts), t(0)
    {}
    bool operator()(Grid<WaveCell> * /*field_swap*/, Grid<WaveCell> * field, double step_t)
    {
        double r = amplitude * cos(t * frequency);
        double w = tilt_zero + t * tilt_speed;
        t += step_t;

        size_t i = field->h * placement.y;
        size_t j = field->w * placement.x;
        field->cell(i, j).velocity = XY(cos(w), sin(w)) * r;
        return false;
    }

private:
    double frequency;
    double tilt_zero;
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


int main()
{
    std::srand(std::time(0));

    size_t q = 7;
    size_t h = 128;
    size_t w = 128;
    Picture pic("img");  //will be flipped by p, and used by indicator as well
    PictureParameters p(pic, h, w);
    std::vector<WaveFlowFunction *> functions;
    for (size_t k = 0; k < q; ++k) {
        XY placement(rnd(1), rnd(1));
        double a = rnd(1) + 0.5;
        double f = 1 / (rnd(7) + 1);
        double tz = rnd(3.14);
        double ts = rnd(0.2) - 0.1;
        functions.push_back(new SimpleFunction(placement, a, f, tz, ts));
    }
    functions.push_back(new EdgeFunction<WaveCell>());
    CompositeFunction<WaveCell> f(functions);
    WaveFlowAnimation a(h, w, p, f);
    SqueezeIndicator indicator(pic, h * 4, w * 4);
    a.run(500, 2, 8, indicator);
    for (size_t k = 0; k <=/*counting edge-function*/ q; ++k) delete functions[k];
}

