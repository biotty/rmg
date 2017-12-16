//      © Christian Sommerfeldt Øien
//      All rights reserved

#include <planar.hpp>
#include <linear.hpp>

#include <unistd.h>
#include <cstdlib>
#include <cassert>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <vector>
#include <stack>
#include <complex>
#include <algorithm>
#include <memory>


XY onto_square(XY p, XY a, XY b)
{
    // note: p in unit, and is transformed onto square (a, b)
    return XY(
        b.x * p.x + a.x * (1 - p.x),
        b.y * p.y + a.y * (1 - p.y));
}


XY onto_unit(XY p, XY a, XY b)
{
    // note: inverse of onto_square (a, b)
    return XY(
        (p.x - a.x) / (b.x - a.x),
        (p.y - a.y) / (b.y - a.y));
}


struct JI
{
    short j;
    short i;

    JI(int j, int i) : j(j), i(i) {}

    operator XY() const { return XY(j, i); }
    bool operator<(const JI & other) const
    {
        return i < other.i || (i == other.i && j < other.j);
    }
};


class PixelTransform
{
    const double m;
public:
    PixelTransform(XY prev_a, XY prev_b, XY a, XY b)
        : m((prev_b.x - prev_a.x) / (b.x - a.x))
    {}

    std::vector<JI> operator()(const std::vector<JI> & pixel_xys, int width, int height)
    {
        const XY r(width, height);
        const XY c = r * .5;
        std::vector<JI> result_xys;
        for (auto e : pixel_xys) {
            XY p(e.j + .5, e.i + .5);  // quantity: center of pixel
            p -= c;
            p *= m;
            p += c;
            const int i = round(p.y);
            const int j = round(p.x);
            if (i >= 0 && i < r.y && j >= 0 && j < r.x) {
                result_xys.push_back(JI(j, i));
            }
        }
        return result_xys;
    }
};


struct Fractal
{
    virtual ~Fractal() {};

    typedef unsigned short value_type;

    const XY a;
    const XY b;
    const int n;
    bool binary;
    Fractal(XY a, XY b, int n, bool binary)
        : a(a) , b(b), n(n), binary(binary)
    {}

    virtual value_type value(XY p) const = 0;
};


struct Mandel : Fractal
{
    Mandel(XY a, XY b, int n, bool binary) : Fractal(a, b, n, binary) {}

    value_type value(XY p) const
    {
        const XY q = onto_square(p, a, b);
        const std::complex<double> c(q.x, q.y);
        std::complex<double> z = 0;
        for (int i=0; i<n; i++) {
            z = z * z + c;
            if (std::abs(z) > 2) {
                return binary ? 1 : n - i;
            }
        }
        return 0;
    }
};


struct Julia : Fractal
{
    const std::complex<double> j;

    Julia(XY j, XY a, XY b, int n, bool binary)
        : Fractal(a, b, n, binary)
        , j(j.x, j.y)
    {}

    value_type value(XY p) const
    {
        const XY q = onto_square(p, a, b);
        std::complex<double> z(q.x, q.y);
        for (int i=0; i<n; i++) {
            z = z * z + j;
            if (std::abs(z) > 2) {
                return binary ? 1 : n - i;
            }
        }
        return 0;
    }
};


struct Block
{
    const JI a;
    const JI b;
    Fractal::value_type a_value;
    Fractal::value_type b_value;
    Fractal::value_type m_value;
    bool of_interest;
    bool env_checked;
    bool wk_interest;

    Block(JI a, JI b,
            Fractal::value_type v0, Fractal::value_type v1,
            Fractal::value_type v2, Fractal::value_type v3)
        : a(a)
        , b(b)
        , of_interest()
        , env_checked()
        , wk_interest()
    {
        of_interest = v0 != v1 || v1 != v2 || v2 != v3;
        a_value = v0;
        b_value = v3;
        const Fractal::value_type v[] = { v0, v1, v2, v3 };
        m_value = *std::max_element(v, v + 4);
    }

    bool contained()
    {
        return a_value == 0 && ! of_interest;
    }
};


struct Image
{
    virtual ~Image() {}

    virtual void put(int b) = 0;
};


struct Grid
{
    const std::unique_ptr<Fractal> f;
    const int width;
    const int height;
    const short s;
    const short n_rows;
    const short n_columns;
    std::vector<JI> interest;

    Block & block(int i, int j)
    {
        return blocks[i * n_columns + j];
    }

    Grid(std::unique_ptr<Fractal> && _f, int width, int height, int s,
            const std::vector<JI> & edge)
        : f(std::move(_f))
        , width(width)
        , height(height)
        , s(s)
        , n_rows(height / s)
        , n_columns(width / s)
        , max_value()
    {
        const XY scale(1 /(double) n_columns, 1 /(double) n_rows);
        std::vector<Fractal::value_type> m;
        m.reserve(n_rows * n_columns);
        for (int i=0; i<n_rows; i++) {
            for (int j=0; j<n_columns; j++) {
                m.push_back(f->value(XY(j, i) * scale));
            }
        }
        for (int i=0; i<n_rows; i++) {
            const int si = s * i;
            for (int j=0; j<n_columns; j++) {
                const int sj = s * j;
                const int ii = i == n_rows - 1 ? i : i + 1;
                const int jj = j == n_columns - 1 ? j : j + 1;

                const auto v0 = m[n_columns * i + j];
                const auto v1 = m[n_columns * i + jj];
                const auto v2 = m[n_columns * ii + j];
                const auto v3 = m[n_columns * ii + jj];
                Block blk(JI(sj, si), JI(sj + s, si + s), v0, v1, v2, v3);
                if (blk.m_value > max_value) max_value = blk.m_value;
                if (blk.of_interest) {
                    interest.push_back(JI(j, i));
                }
                blocks.push_back(blk);
            }
        }
        for (auto e : edge) {
            const int i = e.i / s;
            const int j = e.j / s;
            assert(i < n_rows);
            assert(j < n_columns);
            Block & b = block(i, j);
            if ( ! b.of_interest) {
                b.of_interest = true;
                interest.push_back(JI(j, i));
            }
        }
        std::sort(interest.begin(), interest.end());

        while (check())
            ;
    }

    bool similar_to(const Grid & other, double similarity)
    {
        const int n = interest.size() + other.interest.size();
        std::vector<JI> intersection;
        std::set_intersection(interest.begin(), interest.end(),
                other.interest.begin(), other.interest.end(),
                std::back_inserter(intersection));
        const int c = intersection.size();
        return n == 0 || c > n / (3 - similarity);
    }

    bool points_check(int value, const std::vector<JI> & points)
    {
        const JI r(width, height);
        for (const JI & point : points) {
            XY p = onto_unit(point, xy::zero, r);
            if (f->value(p) != value) {
                return true;
            }
        }
        return false;
    }


    bool neighbor_check(const JI adj, const JI base)
    {
        int i = base.i;
        int j = base.j;
        Block & blk = block(i, j);
        i += adj.i;
        j += adj.j;
        assert(adj.i == 0 || adj.j == 0);
        if (adj.i && (i < 0 || i >= n_rows)) return false;
        if (adj.j && (j < 0 || j >= n_columns)) return false;
        Block & nbr = block(i, j);
        if (nbr.of_interest) return false;

        const JI * corner;
        Fractal::value_type corner_value;
        if (adj.i + adj.j < 0) {
            corner = &blk.a;
            corner_value = blk.a_value;
        } else {
            corner = &blk.b;
            corner_value = blk.b_value;
        }

        std::vector<JI> r;
        if (adj.j == 0) {
            for (int j=blk.a.j; j<blk.b.j; j++) {
                r.push_back(JI(j, corner->i));
            }
        } else {
            for (int i=blk.a.i; i<blk.b.i; i++) {
                r.push_back(JI(corner->j, i));
            }
        }

        if ( ! points_check(corner_value, r)) {
            return false;
        }

        nbr.of_interest = true;
        nbr.wk_interest = true;
        return true;
    }

    bool check()
    {
        std::vector<JI> discovered;
        for (auto q : interest) {
            const int i = q.i;
            const int j = q.j;
            Block & blk = block(i, j);
            if ( ! blk.env_checked) {
                blk.env_checked = true;
                if (neighbor_check(JI(-1, 0), q)) discovered.push_back(JI(q.j - 1, q.i));
                if (neighbor_check(JI(+1, 0), q)) discovered.push_back(JI(q.j + 1, q.i));
                if (neighbor_check(JI(0, -1), q)) discovered.push_back(JI(q.j, q.i - 1));
                if (neighbor_check(JI(0, +1), q)) discovered.push_back(JI(q.j, q.i + 1));
            }
        }
        if (discovered.size() == 0) return false;

        std::copy(discovered.begin(), discovered.end(), std::back_inserter(interest));
        return true;
    }
    
    void generate(Image & image)
    {
        const JI r(width, height);
        for (int y=0; y<height; y++) {
            const int i = y / s;
            for (int x=0; x<width; x++) {
                const Block & blk = block(i, x / s);
                image.put(
                        (
                         ! blk.of_interest
                         ? blk.m_value
                         : f->value(onto_unit(XY(x, y), xy::zero, r)))
                        * 255 /(double) max_value);
            }
        }
    }

    Fractal::value_type max_value;
private:
    std::vector<Block> blocks;
};


struct Frame
{
    virtual ~Frame() {};

    std::unique_ptr<Grid> g;
    XY a;
    XY b;
    XY * fixed;

    Frame() : fixed() {}

    virtual std::unique_ptr<Fractal> fractal(int n, bool binary) = 0;
    virtual std::unique_ptr<Frame> child(double similarity) =  0;

    void orient(XY c, double z, int width, int height)
    {
        const double w = z * width / (double)height;
        const double h = z;
        a = c - XY(w, h) * .5;
        b = a + XY(w, h);
    }

    void orient(XY c, double z) { orient(c, z, g->width, g->height); }

    void setup(int width, int height, int n, int s, double similarity)
    {
        --n;
        std::unique_ptr<Grid> prev_g;
        for (;;) {
            g.reset(new Grid(fractal(n, true), width, height, s, {}));
            if (prev_g && prev_g->similar_to(*g.get(), similarity))
                return;
            ++n;
            // ^ optim: could jmp exp'ly (or 10) on adjacent pairs
            //          and then bisect back when hit similar pair
            prev_g = std::move(g);
        }
    }

    double aspect()
    {
        return g->width / g->height;
    }

    void reset(XY c, double z, int n, const std::vector<JI> & prev_edge)
    {
        XY prev_a = a;
        XY prev_b = b;
        orient(c, z);
        auto transform = PixelTransform(prev_a, prev_b, a, b);
        g.reset(new Grid(fractal(n, false), g->width, g->height, g->s,
                transform(prev_edge, g->width, g->height)));
    }

    XY target()
    {
        if (fixed) return *fixed;

        const XY c = (a + b) * .5;
        const XY block_offset = XY(g->s, g->s) * .5;
        const XY c_as_block_pixel = onto_square(
                onto_unit(c, a, b), xy::zero, XY(g->width, g->height))
                - block_offset;  // note: adjusted to just compare with block.a
        double min_pixel_distance = g->width + g->height;  // note: some big
        XY nearest_interesting_pixel;
        bool found_interesting_pixel = false;
        for (JI kk : g->interest) {
            const Block & blk = g->block(kk.i, kk.j);
            if (blk.wk_interest)
                continue;
            const XY block_pixel = blk.a;
            const double pixel_distance = (block_pixel - c_as_block_pixel).abs();
            if (min_pixel_distance > pixel_distance) {
                min_pixel_distance = pixel_distance;
                nearest_interesting_pixel = block_pixel;
                found_interesting_pixel = true;
            }
        }
        if ( ! found_interesting_pixel) {
            throw nullptr;
        }
        nearest_interesting_pixel += block_offset;
        return onto_square(onto_unit(nearest_interesting_pixel,
                    xy::zero, XY(g->width, g->height)),
                a, b);
    }

    void rnd_targets(std::vector<XY> & targets)
    {
        const int n = g->interest.size();
        for (XY & p : targets) {
            XY block_c;
            int i = n;  // note: probably
            while (--i) {
                const JI kk = g->interest[static_cast<int>(rnd(n))];
                const Block & blk = g->block(kk.i, kk.j);
                if ( ! blk.wk_interest) {
                    block_c = static_cast<XY>(blk.a) + XY(g->s, g->s) * .5;
                    break;
                }
            }
            if ( ! i) throw 0;
            p = onto_square(onto_unit(block_c,
                        xy::zero, XY(g->width, g->height)),
                    a, b);
        }
    }

    bool contained() { return g->interest.size() == 0 && g->block(0, 0).a_value == 0; }
    bool lost() { return g->interest.size() == 0 && g->block(0, 0).a_value != 0; }

    double enter(XY & contained)
    {
        if (g->interest.size() == 0) {
            throw 0.0;
        }

        double max_dist = 0;
        Block * most = nullptr;
        for (int i=0; i<g->n_rows; i++) {
            for (int j=0; j<g->n_columns; j++) {
                if (g->block(i, j).contained()) {
                    double dist = g->n_columns + g->n_rows;  // note: some big
                    for (auto kk : g->interest) {
                        const double d = std::hypot<double>(kk.i - i, kk.j - j);
                        if (dist > d)
                            dist = d;
                    }
                    if (max_dist < dist) {
                        max_dist = dist;
                        most = &g->block(i, j);
                    }
                }
            }
        }
        if ( ! most) return 0;
        contained = onto_square(onto_unit(
                    (static_cast<XY>(most->a)
                     + static_cast<XY>(most->b)) * .5,
                    xy::zero, XY(g->width, g->height)),
                a, b);
        return max_dist;
    }
};              


struct MandelFrame : Frame
{
    MandelFrame(int width, int height, XY c, double z, int s, int n, double similarity)
    {
        orient(c, z, width, height);
        setup(width, height, n, s, similarity);
    }

    std::unique_ptr<Fractal> fractal(int n, bool binary)
    {
        return std::unique_ptr<Fractal>(new Mandel(a, b, n, binary));
    }

    std::unique_ptr<Frame> child(double similarity)
    {
        return std::unique_ptr<Frame>(new MandelFrame(
                    g->width, g->height, target(),
                    (b.y - a.y) * .5, g->s, g->f->n, similarity));
    }
};


struct JuliaFrame : Frame
{
    XY j;

    JuliaFrame(XY j, int width, int height, XY c, double z, int s, int n, double similarity)
        : j(j)
    {
        orient(c, z, width, height);
        setup(width, height, n, s, similarity);
    }

    std::unique_ptr<Fractal> fractal(int n, bool binary)
    {
        return std::unique_ptr<Fractal>(new Julia(j, a, b, n, binary));
    }

    std::unique_ptr<Frame> child(double similarity)
    {
        return std::unique_ptr<Frame>(new JuliaFrame(j,
                    g->width, g->height, target(),
                    (b.y - a.y) * .5, g->s, g->f->n, similarity));
    }
};
        

struct PnmImage : Image
{
    PnmImage(int width, int height)
    {
        std::cout << "P5\n" << width << " " << height
            << "\n255" << std::endl;
    }

    void put(int b)
    {
        std::cout << static_cast<char>(b >= 255 ? 255 : b);
    }
};


struct Tee : Image
{
    Image & s;
    Image & t;
    Tee(Image & s, Image & t) : s(s), t(t) {}

    void put(int b)
    {
        s.put(b);
        t.put(b);
    }
};


struct EdgeDetect : Image
{
    const int width;
    const int height;
    std::vector<JI> edge;
    int i;
    std::vector<bool> prev_row;
    std::vector<bool> row;
    std::vector<bool> curr_row;

    EdgeDetect(int width, int height)
        : width(width), height(height), i()
    {}

    bool on_edge(bool up, bool left, bool m, bool right, bool down)
    {
        return ! m && (up || left || right || down);
    }

    void put(int b)
    {
        curr_row.push_back(b != 0);
        if (curr_row.size() == static_cast<size_t>(width)) {
            if (i > 1 && (i & 1) == 0) {
                assert(i < height);
                for (int j = 2; j < width - 1; j += 2) {
                    if (on_edge(prev_row[j], row[j-1], row[j], row[j+1], curr_row[j])) {
                        edge.push_back(JI(j, i));
                    }
                }
            }
            row.swap(prev_row);
            row.swap(curr_row);
            curr_row.clear();
            ++i;
        }
    }
};


struct FractalMovie
{
    static const int RETRIES_PER_CHILD = 3;
    std::unique_ptr<Frame> frame;
    const double zi;
    std::vector<int> counts;
    int entered;
    
    FractalMovie(std::unique_ptr<Frame> && _frame, int w, double similarity)
        : frame(std::move(_frame))
        , zi(frame->b.y - frame->a.y)
        , entered()
    {
        counts.push_back(frame->g->f->n);
        int i = 1;

        const int k = std::log2(frame->g->n_rows);
        const int i_break = w;
        const int i_enter = w - k;
        const int i_break_entered = w + k;
        std::cerr << w << std::endl
            << i << " " << frame->g->f->n << " iters" << std::endl;
        double enter = 0;
        XY fixed;
        struct retry {
            int i;
            XY c;
            double z;
        };
        bool retrying = false;
        std::stack<retry> retries;
        while ( ! frame->contained()) {
            const bool lost = frame->lost();
            if (( ! entered && i >= i_break)
                    || (entered && i >= i_break_entered) || lost) {
                if (retries.empty()) throw "";
                if ( ! retrying) {
                    while (retries.top().i >= i_enter) retries.pop();
                    retrying = true;
                }
                auto r = retries.top();
                counts.resize(i = r.i);
                frame->orient(r.c, r.z);
                frame->g.reset(new Grid(frame->fractal(counts.back(), true),
                            frame->g->width, frame->g->height, frame->g->s, {}));
                retries.pop();
                std::cerr << i << ":RETRY";
                if (lost) {
                    std::cerr << " ^LOST";
                }
                std::cerr << " target " << std::fixed << std::setprecision(9)
                    << r.c.x << std::showpos << r.c.y << std::noshowpos
                    << "i" << std::endl /* << std::defaultfloat */;
                enter = entered = 0;
            }

            if ( ! retrying) {
                const double z = frame->b.y - frame->a.y;
                std::vector<XY> alts(RETRIES_PER_CHILD);
                frame->rnd_targets(alts);
                for (auto c : alts) {
                    retries.push({i, c, z});
                }
            }

            if (i >= i_enter) {
                XY r;
                if (double d = frame->enter(r)) {
                    if ( ! entered) {
                        entered = i;
                    }
                    if (enter < d) {
                        enter = d;
                        fixed = r;
                    }
                }

                if (entered) {
                    frame->fixed = &fixed;
                }
            }
            std::unique_ptr<Frame> prev = std::move(frame);
            frame = prev->child(entered ? similarity * .6 : similarity);
            XY c = (frame->a + frame->b) * .5;
            counts.push_back(frame->g->f->n);
            i = counts.size();
            std::cerr << i << " (" << std::scientific
                << std::setprecision(3) << (frame->b.y - frame->a.y)
                << ") " << frame->g->f->n << " iters" << std::fixed;
            if (entered) {
                std::cerr << " enter " << std::setprecision(2) << enter;
            }
            std::cerr << " target " << std::setprecision(9) << c.x << std::showpos
                << c.y << std::noshowpos << "i" << std::endl
                /* << std::defaultfloat */;
        }
    }

    void generate(int k)
    {
        XY c = (frame->a + frame->b) * .5;
        const double ang = std::atan2(c.y, c.x);
        XY igp = XY(std::cos(ang) * 2, std::sin(ang) * (1 + frame->aspect())) * 1.2;
        //  quantity: factor is dependent of subject fractal, to get outside it ^
        XY igq = igp - c;
        const double igz = .01;
        const int u = counts.size() - 1;
        std::vector<JI> edge;
        int n = 0; // <-- tech: only to silence compiler warn
        int h = 0; //           ^
        int fixed_max = 0;
        std::cerr << (k*u) << std::endl;
        for (int i=0; i<u; i++) {
            if (i < entered) {
                n = counts[i];
                h = counts[i + 1] - n;
            } else if (i == entered) {
                fixed_max = frame->g->max_value;
                n = counts[entered];
                h = 0;
            }
            for (int j=0; j<k; j++) {
                const double d = j /(double) k;
                const double x = i + d;
                const double z = zi * std::pow(.5, x);
                const int count = n + static_cast<int>(h * d);
                std::cerr << (k*i+j) << " " << count << "\r";
                XY dc = xy::zero;
                if (z > igz) {
                    const double b = (z - igz) / (zi - igz);
                    dc = igq * (b * b);
                }
                frame->reset(c + dc, z, count, edge);
                if (fixed_max) {
                    frame->g->max_value = fixed_max;
                }
                PnmImage img(frame->g->width, frame->g->height);
                EdgeDetect e(frame->g->width, frame->g->height);
                Tee tee(e, img);
                frame->g->generate(tee);
                edge = std::move(e.edge);
            }
        }
        std::cerr << std::endl;
    }
};


int main()
{
    if (isatty(1)) {
        std::cerr << "Please redirect stdout" << std::endl;
        return 2;
    }

    bool do_mandel = false;  // note: mandel or julia

    int width = 1280;  // <-- pixel resolution
    int height = 720;  // <-- ^ y

    double similarity = .99;
    int block_side = 16;   // granularity for grid used to seach border
    int zoom_steps = 32;   // zoom to half frame width this total count
    int images_per_step = 24;   // generates images each half zoom step

    std::srand(std::time(nullptr));

    std::cerr << "Initiating focus on random location" << std::endl;
    XY mc = XY(rnd(2) - 1, rnd(2) - 1);  // mandelbrot frame center
    double mh = 2;  // mandelbrot initial frame height in complex plane
    XY jc = XY(rnd(2) - 1, rnd(2) - 1);  // julia-set ^
    double jh = 2;  // julia-set ^

    std::cerr << "Localizing Mandelbrot coordinate" << std::endl;
    FractalMovie m(std::unique_ptr<Frame>(
                new MandelFrame(width, height, mc, mh, block_side, 30, .6)),
            do_mandel ? zoom_steps : .4 * zoom_steps,
            do_mandel ? similarity : .9 * similarity);
    if (do_mandel) {
        m.generate(images_per_step);
    } else {
        std::cerr << "Using location for Julia Set" << std::endl;
        FractalMovie j(std::unique_ptr<Frame>(
                    new JuliaFrame(m.frame->a, width, height, jc, jh, block_side, 90, .5)),
                zoom_steps, similarity);
        j.generate(images_per_step);
    }
}
