#include "rayt.hpp"

#include <map>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace rayt;

namespace {
    
double rnd()
{
    return static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX);
}

direction rnd_direction()
{
    const double u = rnd();
    const double v = rnd();
    const double theta = 2 * pi * u;
    const double phi = acos(2 * v - 1);
    using namespace std;
    return {
        cos(theta) * sin(phi),
        sin(theta) * sin(phi),
        cos(phi)
    };
}

class state
{
    rotation complete_rot;
    double init_angle;
public:
    const double r;

    state()
        : complete_rot{ rnd_direction(), tau * (rnd() + .5) * 60 }
        , init_angle{ rnd() * tau }  // ^ improve: dep on n-frames
        , r{ rnd() * .0065 + .0085 }
    {}

    void orient(inter & si, double t)
    {
        rotation u{
            complete_rot.axis,
            init_angle + t * complete_rot.angle};
        rot_(si, o, u);
        mul_(si, o, r);
    }
};

void factory(inter & si, int kind)
{
    using namespace solids;
    double cr = 0.123;
    switch (kind) {
    case 1:
        si.push_back(sphere{o, cr = tetra_cr}); cr *= .9;
        for (auto d : tetra_faces) si.push_back(plane{o + d, d});
        break;
    case 2:
        si.push_back(sphere{o, cr = cube_cr}); cr *= .8;
        for (auto d : cube_faces) si.push_back(plane{o + d, d});
        break;
    case 3:
        si.push_back(sphere{o, cr = cubocta_cr}); cr *= .9;
        for (auto d : cubocta_faces) si.push_back(plane{o + d, d});
        break;
    case 4:
        si.push_back(sphere{o, cr = truncocta_cr});
        for (auto d : truncocta_faces) si.push_back(plane{o + d, d});
        break;
    case 5:
        si.push_back(sphere{o, cr = dodeca_cr});
        for (auto d : dodeca_faces) si.push_back(plane{o + d, d});
        break;
    case 6:
        si.push_back(sphere{o, cr = octa_cr}); cr *= 1.2;
        for (auto d : octa_faces) si.push_back(plane{o + d, d});
        break;
    }
    mul_(si, o, 1 / cr);
}

struct wgen {
    wgen(double s, double w) : tr_s(s), tr_w(w) {}
    world operator()(double seqt);
    int i = 0;
    const double tr_s;
    const double tr_w;
    std::string data_path;
    std::map<std::string, state> states = {};
private:
    void redim(double & x, double & y) {
        x -= .5; y -= .5; x *= tr_w; y = -y;
    }
};

static bool file_exists(const char * path)
{
    std::ifstream infile(path);
    return infile.good();
}

world wgen::operator()(double seqt)
{
    std::function<color(direction)> sky = [seqt](direction d) -> color
    {
            double a = atan2(d.x, d.y) + tau * (.9 + seqt * .6);
            double s = d.x > 0 ? .8 : .6;
            double v = d.z < -.1 ? .4 : .8;
            return from_hsv(a, s, v);
    };
    const char * sky_path = "sky.jpeg";
    if (file_exists(sky_path)) {
        sky = photo_sky(sky_path);
    }

    std::ostringstream oss;
    oss << data_path << i++ << ".txt";
    std::ifstream f(oss.str());

    struct glass : optics {
        glass(double h, double s, double r)
        : optics{
            from_hsv(h, .3 * s, .08),
            from_hsv(h, .4 * s, .06), glass_ri,
            from_hsv(h, .8 * s, .88),
            from_hsv(h, .8 * s, .92 * r)}
        {}
    };

    world wr{
        observer{ ony(tr_w), o, xd * tr_s },
        sky, {}, {{{ 6, 0, 9 }, white }}
    };
    std::string line;
    double tr_x, tr_y;
    if (std::getline(f, line)) {
        std::istringstream iss(line);
        iss >> tr_x >> tr_y;
        redim(tr_x, tr_y);
    }
    double sp_x, sp_y, sp_r;
    if (std::getline(f, line)) {
        std::istringstream iss(line);
        iss >> sp_x >> sp_y >> sp_r;
        redim(sp_x, sp_y);
    }
    if ( ! f) std::cerr << "bad input\n";
    while (std::getline(f, line)) {
        std::istringstream iss(line);
        std::string id;
        double x, y, hue;
        int kind;
        iss >> id >> x >> y >> kind >> hue;
        redim(x, y);

        if (std::abs(x - tr_x) < 1.2 * tr_w * tr_s
                && std::abs(y - tr_y) < 1.2 * tr_s) {
            double r = states[id].r;
            object poly{ {}, glass{hue, 1, r}, {} };
            factory(poly.si, kind);
            states[id].orient(poly.si, seqt);
            mov_(poly.si, direction{x, 0, y});
            wr.s.push_back(poly);
        }
    }
    const double s = sp_r * .14;
    object ball{
        {
            sphere{ {sp_x, 0, sp_y}, sp_r + s },
            inv_sphere{ {sp_x, 0, sp_y}, sp_r - s },
            plane{ony(s), yd},
            inv_plane{ony(-s), yd}
        },
        glass{ 0, 0, 1 }, {}
    };
    wr.s.push_back(ball);
    wr.obs.c = {tr_x, 0, tr_y};
    return wr;
}

}

int main(int argc, char ** argv)
{
    auto a{ args(argc, argv, "D") };
    std::srand(std::time(nullptr));
    wgen g{
        .1, a.r.width /(double) a.r.height
    };
    g.data_path = a.get('D');
    a.run(g);
}
