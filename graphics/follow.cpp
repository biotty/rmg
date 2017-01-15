
#include <planar.hpp>
#include <linear.hpp>


#include <cstdlib>
#include <cassert>
#include <ctime>
#include <iostream>
#include <vector>
#include <complex>
#include <algorithm>
#include <memory>


XY onto_square(XY self, XY a, XY b)
{
    // self is in unit, and is transformed onto square (a, b)
    return XY(
        b.x * self.x + a.x * (1 - self.x),
        b.y * self.y + a.y * (1 - self.y));
}


XY onto_unit(XY self, XY a, XY b)
{
    // inverse of onto_square (a, b)
    return XY(
        (self.x - a.x) / (b.x - a.x),
        (self.y - a.y) / (b.y - a.y));
}


struct JI
{
    const int j;
    const int i;

    JI(int j, int i) : j(j), i(i) {}

    operator XY() const { return XY(j, i); }
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

    const XY a;
    const XY b;
    const int n;
    bool binary;
    Fractal(XY a, XY b, int n, bool binary)
        : a(a) , b(b), n(n), binary(binary)
    {}

    virtual int value(XY p) const = 0;
};


struct Mandel : Fractal
{
    Mandel(XY a, XY b, int n, bool binary) : Fractal(a, b, n, binary) {}

    int value(XY p) const
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

    int value(XY p) const
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
    int a_value;
    int b_value;
    int m_value;
    bool of_interest;
    bool env_checked;
    bool wk_interest;

    Block(JI a, JI b)
        : a(a)
        , b(b)
        , of_interest()
        , env_checked()
        , wk_interest()
    {}

    void setup(const Fractal & f, int width, int height)
    {
        const JI r(width, height);
        const XY c0 = onto_unit(a, xy::zero, r);
        const XY c3 = onto_unit(b, xy::zero, r);
        const XY c1 = XY(c3.x, c0.y);
        const XY c2 = XY(c0.x, c3.y);
        int v[] = { f.value(c0), f.value(c1), f.value(c2), f.value(c3) };
        of_interest = v[0] != v[1] || v[1] != v[2] || v[2] != v[3];
        m_value = *std::max_element(v, v + 4);
        a_value = v[0];
        b_value = v[3];
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
    const int s;
    const int n_rows;
    const int n_columns;
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
        for (int i=0; i<n_rows; i++) {
            const int y = i * s;
            for (int j=0; j<n_columns; j++) {
                const int x = j * s;
                blocks.push_back(Block(JI(x, y), JI(x + s, y + s)));
            }
        }

        for (int i=0; i<n_rows; i++) {
            for (int j=0; j<n_columns; j++) {
                Block & b = block(i, j);
                b.setup(*f.get(), width, height);
                // improve: each corner is now calculated four times
                //          -- rather share calc to set them at once
                if (b.m_value > max_value) max_value = b.m_value;
                if (b.of_interest) {
                    interest.push_back(JI(j, i));
                }
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

        while (check())
            ;
    }

    bool similar_to(const Grid & other, double similarity)
    {
        auto cmp = [](const JI & a, const JI & b)
        { return a.i < b.i || (a.i == b.i && a.j < b.j); };
        const int n = interest.size() + other.interest.size();
        std::vector<JI> intersection;
        std::set_intersection(interest.begin(), interest.end(),
                other.interest.begin(), other.interest.end(),
                std::back_inserter(intersection), cmp);
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
        int corner_value;
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

private:
    std::vector<Block> blocks;
    int max_value;
};


struct Frame
{
    virtual ~Frame() {};

    std::unique_ptr<Grid> g;
    XY a;
    XY b;
    XY c;
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

    void setup(int width, int height, int n, int s, double similarity)
    {
        --n;
        std::unique_ptr<Grid> prev_g;
        for (;;) {
            g.reset(new Grid(fractal(n, true), width, height, s, {}));
            if (prev_g && prev_g->similar_to(*g.get(), similarity))
                return;
            ++n;
            prev_g = std::move(g);
        }
    }

    double aspect()
    {
        return (b.x - a.x) / (b.y - a.y);
    }

    void reset(XY c, double z, int n, const std::vector<JI> & prev_edge)
    {
        XY prev_a = a;
        XY prev_b = b;
        orient(c, z, g->width, g->height);
        auto transform = PixelTransform(prev_a, prev_b, a, b);
        g.reset(new Grid(fractal(n, false), g->width, g->height, g->s,
                transform(prev_edge, g->width, g->height)));
    }

    XY target()
    {
        if (fixed) return *fixed;

        c = (a + b) * .5;
        XY block_offset = XY(g->s, g->s) * .5;
        XY c_as_block_pixel = onto_square(
                onto_unit(c, a, b), xy::zero, XY(g->width, g->height))
                - block_offset;  // adjusted to just compare with block.a
        double min_pixel_distance = g->width + g->height;  // quantity: some big
        XY nearest_interesting_pixel;
        bool found_interesting_pixel = false;
        for (JI kk : g->interest) {
            Block & blk = g->block(kk.i, kk.j);
            if (blk.wk_interest)
                continue;
            XY block_pixel = blk.a;
            double pixel_distance = (block_pixel - c_as_block_pixel).abs();
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

    bool contained()
    {
        return g->block(0, 0).a_value == 0 && g->interest.size() == 0;
    }

    double enter(XY & contained)
    {
        if (g->interest.size() == 0) return 0;

        double max_dist = 0;
        Block * most = nullptr;
        for (int i=0; i<g->n_rows; i++) {
            for (int j=0; j<g->n_columns; j++) {
                if (g->block(i, j).contained()) {
                    double dist = g->n_columns + g->n_rows;  // quantity: some big
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
    std::unique_ptr<Frame> frame;
    const double zi;
    std::vector<int> counts;
    
    FractalMovie(std::unique_ptr<Frame> && _frame, int w)
        : frame(std::move(_frame))
        , zi(frame->b.y - frame->a.y)
    {
        counts.push_back(frame->g->f->n);
        auto i = 1;
        const int i_enter = w - std::log2(frame->g->n_rows);
        const int i_break = w * 1.2;
        std::cerr << w << std::endl
            << i << " " << frame->g->f->n << " iters" << std::endl;
        double enter = 0;
        XY fixed;
        while ( ! frame->contained() && i < i_break) {
            if (i >= i_enter) {
                XY r;
                if (double d = frame->enter(r)) {
                    if (enter < d) {
                        enter = d;
                        fixed = r;
                    }
                }

                if (enter) {
                    frame->fixed = &fixed;
                }
            }
            std::unique_ptr<Frame> prev = std::move(frame);
            frame = prev->child(enter ? .4 : .993);
            XY c = (frame->a + frame->b) * .5;
            counts.push_back(frame->g->f->n);
            i = counts.size();
            std::cerr << i << " ("
                << std::scientific << (frame->b.y - frame->a.y) << ") ";
            std::cerr.unsetf(std::ios_base::floatfield);
            std::cerr << frame->g->f->n << " iters";
            if (enter) std::cerr << " enter " << enter;
            std::cerr << " target " << c.x << std::showpos
                << c.y << std::noshowpos << "i" << std::endl;
        }
        if (i == i_break) throw "broken";
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
        for (int i=0; i<u; i++) {
            const int n = counts[i];
            const int h = counts[i + 1] - n;
            for (int j=0; j<k; j++) {
                const double d = j /(double) k;
                const double x = i + d;
                const double z = zi * std::pow(.5, x);
                const int cnt = n + static_cast<int>(h * d);
                std::cerr << (i*k+j) << "/" << (u*k) << " count " << cnt << "\r";
                std::cerr.unsetf(std::ios_base::floatfield);
                XY dc = xy::zero;
                if (z > igz) {
                    const double b = (z - igz) / (zi - igz);
                    dc = igq * (b * b);
                }
                frame->reset(c + dc, z, cnt, edge);
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
    extern int isatty(int fd);
    if (isatty(1)) {
        std::cerr << "Please redirect stdout" << std::endl;
        return 2;
    }

    // note: mandel or julia
    bool do_mandel = true;

    int width = 1280;
    int height = 720;

    int block_side = 16;  // granularity for grid used to seach border
    int zoom_steps = 32;  // zoom to half frame width this total count
    int images_per_step = 24;

    std::srand(std::time(nullptr));

    std::cerr << "Initiating focus on random location" << std::endl;
    XY mc = XY(rnd(2) - 1, rnd(2) - 1);  // mandelbrot frame center
    double mh = 2;  // mandelbrot initial frame height in complex plane
    XY jc = XY(rnd(2) - 1, rnd(2) - 1);  // julia-set ^
    double jh = 2;  // julia-set ^

    std::cerr << "Localizing Mandelbrot coordinate" << std::endl;
    FractalMovie m(std::unique_ptr<Frame>(
                new MandelFrame(width, height, mc, mh, block_side, 32, .5)),
            zoom_steps);
    if (do_mandel) {
        m.generate(images_per_step);
    } else {
        std::cerr << "Using location for Julia Set" << std::endl;
        FractalMovie j(std::unique_ptr<Frame>(
                    new JuliaFrame(m.frame->a, width, height, jc, jh, block_side, 32, .5)),
                zoom_steps);
        j.generate(images_per_step);
    }
}
