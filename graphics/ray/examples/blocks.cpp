#include "rayt.hpp"
#include "sky.h"

#include <vector>
#include <limits>
#include <cstdlib>
#include <ctime>
#include <iostream>

using namespace rayt;
using namespace std;

namespace {

double randd()
{
  return (double)rand() / ((double)RAND_MAX + 1);
}

unsigned rands(unsigned n)
{
    return static_cast<unsigned>(floor(randd() * n));
}

#define N_PLASTICS 10
struct {
    unsigned h, s, v;
} plastics[N_PLASTICS] = {
    {  0, 1, 1},
    {151, 0, 2},
    {242, 2, 1},
    { 51, 1, 1},
    {208, 2, 0},
    {315, 2, 2},
    { 82, 0, 2},
    { 26, 0, 1},
    {208, 0, 1},
    {266, 4, 0},
};

struct plastic_optics : optics {
    enum class location { inner, outer };
    plastic_optics(unsigned i, location y, double t) {
        assert(i < N_PLASTICS);
        assert(t >= 0);
        assert(t <= 1);
        const double h = plastics[i].h * pi / 180;
        const double s = linear(1 - .1 * plastics[i].s, 0, t);
        const double v = linear(1 - .1 * plastics[i].v, .98, t);
        reflection_filter = from_hsv(h, .15 * s, v * (.15 + .05 * (int)y));
        absorption_filter = black;
        refraction_index = linear(glass_ri, 1, t);
        refraction_filter = from_hsv(h, .55 * s, v * (.95 - .05 * (int)y));
        passthrough_filter = from_hsv(h, .35 * s, v * .85);
    }
};

namespace factory {
    constexpr double height = 4. / 3;
    constexpr double bump_d = 5. / 9;
    constexpr double tiny_d = 1e2 * TINY_REAL;
    constexpr double trunk_r = (M_SQRT2 * 9 - 8) * height / 24;
    constexpr double h_unit = height / 3;

    direction modus_direction(unsigned modus) {
        direction r = { 0, 0, 0 };
        switch(modus & 3) {
        case 0: r.x = 1; break;
        case 1: r.y = 1; break;
        case 2: r.x = -1; break;
        case 3: r.y = -1; break;
        }
        return r;
    }

    struct fixture {
        point p;
        unsigned plastic_i;
        double h;
        unsigned x_n;
        unsigned y_n;
        unsigned modus;
        double e;
        double s;
    };

    void make(vector<object> &blocks, fixture & f, double s)
    {
        if (s < f.p.z + f.e) return;
        if (f.s < 0) return;

        if (f.s == 0) f.s = s;
        const double t = (s - f.s) * .35;
        if (t >= 1) {
            f.s = -1;
            return;
        }

        point c = f.p + zd * (f.h / 2 - s);
        double vt = 0;
        if (t > .8) {
            vt = delinear(.8, 1, t);
        } else if (t < .3) {
            c.z += square(delinear(.3, 0, t) * 4);
            if (t < .2) vt = delinear(.2, 0, t);
        }

        assert(f.x_n && f.y_n);

        const auto optou = plastic_optics{ f.plastic_i, plastic_optics::location::outer, vt };
        const direction mxd = modus_direction(f.modus);
        const direction myd = modus_direction(f.modus + 1);
        const point g = c + mxd * (.5 * (f.x_n - 1)) + myd * (.5 * (f.y_n - 1));

        blocks.push_back({{
            cuboid{g,
                zd, mxd, {f.x_n - tiny_d, f.y_n - tiny_d, f.h - tiny_d}},
            inv_cuboid{g +- zd * (height / 6),
                zd, mxd, {bump_d + (f.x_n - 1), bump_d + (f.y_n - 1), f.h}},
            }, optou, {}});
        for (unsigned y_i = 0; y_i < f.y_n; y_i++) {
            for (unsigned x_i = 0; x_i < f.x_n; x_i++) {
                const point bc = c + mxd * x_i + myd * y_i;
                blocks.push_back({{
                    cylinder{bc, bump_d / 2 - tiny_d, zd},
                    inv_plane{c + zd * (f.h / 2 - tiny_d), zd},
                    plane{c + zd * (f.h / 2 + height / 6 - tiny_d), zd},
                    }, optou, {}});
            }
        }

        const auto optin = plastic_optics{ f.plastic_i, plastic_optics::location::inner, vt };
        for (unsigned y_j = 1; y_j < f.y_n; y_j++) {
            for (unsigned x_j = 1; x_j < f.x_n; x_j++) {
                blocks.push_back({{
                    cylinder{c + mxd * (x_j - .5) + myd * (y_j - .5),
                        trunk_r - tiny_d, zd},
                    inv_plane{c +- zd * (f.h / 2 - tiny_d), zd},
                    plane{c + zd * (f.h / 2 - tiny_d), zd},
                    }, optin, {}});
            }
        }
        if ((f.x_n == 1) != (f.y_n == 1)) {
            direction md = (f.x_n == 1) ? myd : mxd;
            unsigned n = max(f.x_n, f.y_n);
            for (unsigned k = 1; k < n; k++) {
            blocks.push_back({{
                cylinder{c + md * (k - .5), height / 6 - tiny_d, zd},
                inv_plane{c +- zd * (f.h / 2 - tiny_d), zd},
                plane{c + zd * (f.h / 2 - tiny_d), zd},
                }, optin, {}});
            }
        }
    }

    struct reserver {
        const unsigned x_m;
        const unsigned y_m;
        vector<unsigned> h_map;

        reserver(double x_m, double y_m) : x_m(x_m), y_m(y_m)
        {
            h_map.resize(x_m * y_m);
        }

        unsigned index(point p)
        {
            int x_i = round(p.x);
            int y_i = round(p.y);
            assert(x_i >= 0);
            assert(y_i >= 0);
            assert((unsigned)x_i < x_m);
            assert((unsigned)y_i < y_m);

            return y_i * x_m + x_i;
        }

        unsigned get(point p) { return h_map[index(p)]; }

        void set(unsigned h, point p) { h_map[index(p)] = h; }

        struct probe_result {
            unsigned h;
            unsigned count;
        };

        struct probe_arg {
            unsigned x_c, y_c, x_n, y_n, modus;

            point p(unsigned x_i, unsigned y_i)
            {
                assert(x_i < x_n);
                assert(y_i < y_n);
                const direction mxd = modus_direction(modus);
                const direction myd = modus_direction(modus + 1);
                return o + xd * x_c + yd * y_c + mxd * x_i + myd * y_i;
            }
        };

        probe_result probe(probe_arg a)
        {
            assert(a.x_n && a.y_n);
            probe_result r = { 0, 0 };
            for (unsigned y_i = 0; y_i < a.y_n; y_i++) {
                for (unsigned x_i = 0; x_i < a.x_n; x_i++) {
                    const unsigned h = get(a.p(x_i, y_i));
                    if (r.h < h) {
                        r.h = h;
                        r.count = 0;
                    } else if (h == r.h) {
                        ++r.count;
                    }
                }
            }
            return r;
        }

        void take(probe_arg a, unsigned z_n)
        {
            for (unsigned y_i = 0; y_i < a.y_n; y_i++) {
                for (unsigned x_i = 0; x_i < a.x_n; x_i++) {
                    set(z_n, a.p(x_i, y_i));
                }
            }
        }
    };  // end: struct reserver
} // end: namespace factory

struct wgen {
    double eye_phi, eye_tan, r, h;
    vector<factory::fixture> fixtures;
    wgen(unsigned board_n, unsigned n_blocks);
    world operator()(double seqt);
};

wgen::wgen(unsigned board_n, unsigned n_blocks)
{
    r = h = 0;
    using namespace factory;
    eye_phi = randd() * pi * 2;
    eye_tan = .4 * (1 + randd());
    const unsigned block_n = 4;
    reserver rs(board_n + 2 * block_n, board_n + 2 * block_n);
    for (unsigned i = 0; i < n_blocks; i++) {
        const unsigned z_n = (i % 3 == 0) ? 1 : 3;
        unsigned x_n, y_n;
        if (z_n == 1) {
            x_n = 1 + rands(block_n);
            y_n = 1 + rands(block_n);
        } else {
            x_n = 1 + rands(2);
            y_n = 1 + rands(block_n);
        }
        const unsigned b_n = x_n * y_n;

        unsigned r_h = numeric_limits<unsigned>::max();
        reserver::probe_arg best = {};
        for (unsigned compete = 0; compete < 4; compete++) {
            reserver::probe_result rpr = {};
            reserver::probe_arg rpa = {};
            bool good = false;
            for (unsigned insist = 0; insist < board_n; insist++) {
                const unsigned x_c = rands(board_n);
                const unsigned y_c = rands(board_n);
                const unsigned modus = rands(4);
                rpa = { x_c + block_n, y_c + block_n, x_n, y_n, modus };
                rpr = rs.probe(rpa);
                for (int j = b_n; j >= (1 + (int)b_n) / 2; j--) {
                    if (rpr.count == (unsigned)j && rands(1 + b_n - j) == 0) {
                        good = true;
                        goto done;
                    }
                }
            }
done:
            if (good && r_h > rpr.h) {
                r_h = rpr.h;
                best = rpa;
            }
        }
        if (best.x_n == 0) continue;

        const unsigned z_u = r_h + z_n;
        rs.take(best, z_u);
        const double x = (best.x_c - (.5 * board_n + block_n));
        const double y = (best.y_c - (.5 * board_n + block_n));
        const point p = o + xd * x + yd * y + zd * (r_h * h_unit);
        const double e = .65 * randd();

        {
            const double rr = max(abs(p.x), abs(p.y));
            if (r < rr) r = rr;
            const double hh = p.z + e;
            if (h < hh) h = hh;
        }

        fixtures.push_back({
                p, rands(N_PLASTICS),  // alt: hue set to srand(8)
                z_n * h_unit, x_n, y_n, best.modus, e, 0 });
    }
    h += 1 / .35;
}

world wgen::operator()(double seqt)
{
    vector<object> blocks;
    for (auto & f : fixtures) {
        factory::make(blocks, f, seqt * h);
    }

    const double phi = eye_phi + .8 * sin(seqt * 2 * pi);
    const direction right = xyc(phi + pi * .5);
    const point eye = o +- xyc(phi) * r * 2
        + zd * r * .7 * (eye_tan + sin(seqt * 4 * pi));
    const point focus = o +- zd * 1.8;

    return { observer{ eye, focus, right * r }, rgb_sky, blocks, {} };
}

} // namespace ""

int main(int argc, char ** argv)
{
    char *e = getenv("SRAND");
    unsigned s = e ? atoi(e) : time(NULL);
    cout << "SRAND=" << s << "\n";
    srand(s);
    args(argc, argv).run(wgen(9, 99));
}
