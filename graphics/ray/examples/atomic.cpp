#include "rayt.hpp"

#include <map>
#include <sstream>
#include <fstream>
#include <cstdlib>

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
        : complete_rot{ rnd_direction(), rnd() * 19 + 9 }
        , init_angle{ rnd() * tau }
        , r{ rnd() * .006 + .008 }
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
};

world wgen::operator()(double seqt)
{
    std::ostringstream oss;
    oss << data_path << i++ << ".txt";
    std::ifstream f(oss.str());

    world wr{
        observer{ onz(.5), o, xd * tr_s },
        [seqt](direction d) -> color {
            double a = atan2(d.x, d.z) + tau * (.5 + seqt * .6);
            double s = d.x < 0 ? .8 : .6;
            double y = std::abs(d.y);
            double v = y < .05 ? .1 : .9;
            return from_hsv(a, s, v);
        },
        {}, {}
    };
    std::string line;
    double tr_x, tr_y;
    if (std::getline(f, line)) {
        std::istringstream iss(line);
        iss >> tr_x >> tr_y;
        tr_x -= .5; tr_y -= .5; tr_x *= tr_w;
    }
    while (std::getline(f, line)) {
        std::istringstream iss(line);
        std::string id;
        double x, y, hue;
        int kind;
        iss >> id >> x >> y >> kind >> hue;
        x -= .5; y -= .5; x *= tr_w;

        if (std::abs(x - tr_x) < 1.1 * tr_w * tr_s
                && std::abs(y - tr_y) < 1.1 * tr_s) {
            double r = states[id].r;
            object poly{ {}, optics{
                    from_hsv(hue, .5, .09),
                    black, glass_ri,
                    from_hsv(hue, .6, .9),
                    from_hsv(hue, .5, .85 * r)
                }, {} };
            factory(poly.si, kind);
            states[id].orient(poly.si, seqt);
            mov_(poly.si, direction{x, y, 0});
            wr.s.push_back(poly);
        }
    }
    wr.obs.c = {tr_x, tr_y, 0};
    return wr;
}

}

int main(int argc, char ** argv)
{   auto a{ args(argc, argv) };
    wgen g{
        .12, a.r.width /(double) a.r.height
    };
    // improve: ability to pass user-param args
    g.data_path = "a.movie/";
    a.run(g);
}
