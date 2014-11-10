//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "wave.hpp"
#include "linear.hpp"
#include "image.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <ctime>

//
//   # what's ahead #
//
//--description 1--
//make a gradient-sky (set of (color,direction)),
//and a set of (photo,direction) that are mixed in X along into the center,
//*or* striped out (edge-pixel and out from center) X image-size fade-out
//(mix in both cases means with the gradient-background and every other
//photos interaction at that direction.
//
//--description 2--
//have an alternate mode of cray.c -- where no image is traced, but we run
//the trace function for random-direction rays from random-selected spots,
//and one of the decoration-maps must have path specified as "*", meaning
//it is the collector.  each time the decoration-function is called, the
//x,y coordinates that would have been taken from a photo, will instead have
//mixed(added) to a pixel in a result-map (starting at all-black) the
//current filter-color of the calling detector (this is what is left of the
//spots light (if we consider we followed light-direction and not traced it).
//-after a while, or at regular intervals, the collector image is written.
//the mode is simply triggered by the presence of a "[C]*P[!Q]" in a map-path,
//snapshot (path will be P and eventually seceral N.P for each Qth light-shot)
//-the refraction-index of all objects are first set to match red, then we
//do some thousands of photon-emits from all spots, then blue then green.
//
// . make cray mode that collects on a map-photo; see description 2.
// . see description 1.
// ? make an image-filter program in cpp that applies as watermark (gray is neutral) to other image.
// . make photo.c tolerate header-comment (see the py code) and handle ".png".
// . program "spheretrim" that 
// . function in palette converting wavelength to rgb (fortran found in wavelength_to_rgb.txt)
//   -- to be used on cray in collect-mode, see description 2.  the refraction-index-adjustment
//   is to be done based on the wave-length of the color (instead of simply shooting r, g and b).
// 9 indicate with a picture with displaced pixels drawed on a grid like photo-colorizers tracer
//   -- scan over after all is drawn filling-in all black spots (seek down-right to closest hit)
// . functions that moves over both fluid or wave applying friction-force based on velocity diff.
// b have force (i.e. small) that drags displacement back to zero
// c program-parameters that picks (cwave and cfluid) a color to be immovable concrete.
// d enable cwave to take picture from a picture-sequence (can use cfluid-generated files)
// e fixation that applies an acceleration (like gravity (static south), so density-invariant) based on color
//   -- applies to both fluid and waveflow.
// f color-based reactions
//   * cfluid -- two colors giving a third (all three selected by coordinates in original picture)
//               and a slight augmentation in pressure on those two transformed cells.
//               (colors are result of quantize, just like for the fluid-parameters).
//   * cwave -- two colors repelling (we talk about dominant-colors of cell-quantization)
//              like magnets.

// a have independent resolution on output-image todo use Interpolation
// b do -displacement to fetch color (like tracer-move once, but no rnd)
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
            Color c = picture.color(XY(p.j/(double)it.w, p.i/(double)it.h));
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

    Color shaded_color(Color c, double sq)
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
        return intensity(picture.color(u)) * 1.9 + 0.1;
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

