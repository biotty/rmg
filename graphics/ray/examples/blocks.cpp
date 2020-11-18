#include "rayt.hpp"
#include "sky.h"

#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>

using namespace rayt;
using namespace std;

double randd()
{
  return (double)rand() / ((double)RAND_MAX + 1);
}

// note: random utilities are needed, to accompany rayt.hpp
//       or as part of it - the following are also found in
//       the droplets example !
//
struct polar {
    double theta;
    double phi;
};

polar sphere_uniform(double u, double v)
{
    double theta = 2 * pi * u;
    double phi = acos(2 * v - 1);
    return { theta, phi };
}

direction sphere_random()
{
    direction r = zd;
    polar p = sphere_uniform(randd(), randd());
    rot_(r, { xyc(p.phi), p.theta });
    return r;
}
// ^ :note
//

unsigned rands(unsigned n)
{
    return static_cast<unsigned>(floor(randd() * n));
}

struct plastic_optics : optics {
    plastic_optics(double hue) : optics{
        from_hsv(hue, .3, .2),
        black,
        glass_ri,
        from_hsv(hue, .5, .8),
        from_hsv(hue, .4, .7) }
    {}
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
        direction v;
        double hue;
        double h;
        unsigned x_n;
        unsigned y_n;
        unsigned modus;
    };

    void make(vector<object> &blocks, fixture f, double seqt)
    {
        assert(f.x_n && f.y_n);
        const direction mxd = modus_direction(f.modus);
        const direction myd = modus_direction(f.modus + 1);
        const point c = f.p + zd * (f.h / 2) + f.v * square(1 - seqt);
        const point g = c + mxd * (.5 * (f.x_n - 1)) + myd * (.5 * (f.y_n - 1));
        blocks.push_back({{
            cuboid{g,
                zd, mxd, {f.x_n - tiny_d, f.y_n - tiny_d, f.h - tiny_d}},
            inv_cuboid{g +- zd * (height / 6),
                zd, mxd, {bump_d + (f.x_n - 1), bump_d + (f.y_n - 1), f.h}},
            }, plastic_optics(f.hue), {}});
        for (unsigned y_i = 0; y_i < f.y_n; y_i++) {
            for (unsigned x_i = 0; x_i < f.x_n; x_i++) {
                blocks.push_back({{
                    cylinder{c + mxd * x_i + myd * y_i, bump_d / 2 - tiny_d, zd},
                    inv_plane{c + zd * (f.h / 2 - tiny_d), zd},
                    plane{c + zd * (f.h / 2 + height / 6 - tiny_d), zd},
                    }, plastic_optics(f.hue), {}});
            }
        }
        for (unsigned y_j = 1; y_j < f.y_n; y_j++) {
            for (unsigned x_j = 1; x_j < f.x_n; x_j++) {
                blocks.push_back({{
                    cylinder{c + mxd * (x_j - .5) + myd * (y_j - .5),
                        trunk_r - tiny_d, zd},
                    inv_plane{c +- zd * (f.h / 2 + tiny_d), zd},
                    plane{c + zd * (f.h / 2 - tiny_d), zd},
                    }, plastic_optics(f.hue), {}});
            }
        }
        if ((f.x_n == 1) != (f.y_n == 1)) {
            direction md = (f.x_n == 1) ? myd : mxd;
            unsigned n = max(f.x_n, f.y_n);
            for (unsigned k = 1; k < n; k++) {
            blocks.push_back({{
                cylinder{c + md * (k - .5), height / 6 - tiny_d, zd},
                inv_plane{c +- zd * (f.h / 2 + tiny_d), zd},
                plane{c + zd * (f.h / 2 - tiny_d), zd},
                }, plastic_optics(f.hue), {}});
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
    double eye_phi;
    vector<factory::fixture> fixtures;
    wgen(unsigned board_n, unsigned n_blocks);
    world operator()(double seqt);
};

wgen::wgen(unsigned board_n, unsigned n_blocks)
{
    using namespace factory;
    eye_phi = randd() * pi * 2;
    const unsigned block_n = 4;
    reserver rs(board_n + 2 * block_n, board_n + 2 * block_n);
    for (unsigned i = 0; i < n_blocks; i++) {
        const direction v = sphere_random() * board_n;
        const double hue = randd() * pi * 2;
        const unsigned z_n = (i % 3 == 0) ? 1 : 3;
        unsigned x_n, y_n;
        if (z_n == 1) {
            x_n = 1 + rands(block_n);
            y_n = 1 + rands(block_n);
        } else {
            x_n = 1 + rands(2);
            y_n = 1 + rands(block_n);
        }
        unsigned b_n = x_n * y_n;
        reserver::probe_arg a = {};
        reserver::probe_result r = {};
        unsigned x_c, y_c, modus;
        for (unsigned insist = 0; insist < board_n * 4; insist++) {
            x_c = rands(board_n);
            y_c = rands(board_n);
            modus = rands(4);
            a = { x_c + block_n, y_c + block_n, x_n, y_n, modus };
            r = rs.probe(a);
            if (r.count == b_n)
            for (int j = b_n; j >= 0; j--)
                if (r.count == (unsigned)j && rands(1 + b_n - j) == 0)
                    goto e;
        }
e:      if (r.count > b_n / 2) {
            const unsigned z_u = r.h + z_n;
            rs.take(a, z_u);
            double x = (x_c - .5 * board_n);
            double y = (y_c - .5 * board_n);
            point p = o + xd * x + yd * y + zd * (r.h * h_unit);
            fixtures.push_back({ p, v, hue, z_n * h_unit, x_n, y_n, modus });
        }
    }
}

world wgen::operator()(double seqt)
{
    double r = 0;
    vector<object> blocks;
    for (auto f : fixtures) {
        const double rr = max(abs(f.p.x), abs(f.p.y));
        if (r < rr)
            r = rr;
        factory::make(blocks, f, seqt);
    }

    const direction right = xyc(eye_phi + pi * .5);
    return {
        observer{ o +- xyc(eye_phi) * r * 2 + zd * .4 * r * (1 + randd()),
            o, right * r }, rgb_sky,
        blocks,
        {}
    };
}

int main(int argc, char ** argv)
{
    char *e = getenv("SRAND");
    unsigned s = e ? atoi(e) : time(NULL);
    cout << "SRAND=" << s << "\n";
    srand(s);
    args(argc, argv).run(wgen(16, 128));
}
