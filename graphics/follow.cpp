
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
};


struct PixelTransform
{
    const double m;

    PixelTransform(XY prev_a, XY prev_b, XY a, XY b)
        : m((prev_b.x - prev_a.x) / (b.x - a.x))
    {}

    std::vector<JI> operator()(const std::vector<JI> & pixel_xys, int width, int height)
    {
        const XY r(width, height);
        const XY c = r * 0.5;
        std::vector<JI> result_xys;
        for (auto e : pixel_xys) {
            XY p(e.j + 0.5, e.i + 0.5);  // quantity: center of pixel
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
    Fractal(XY min_, XY max_, int n) : a(min_) , b(max_), n(n) {}

    virtual int value(XY p) const = 0;
};


struct Mandel : Fractal
{
    Mandel(XY a, XY b, int n) : Fractal(a, b, n) {}

    int value(XY p) const
    {
        XY q = onto_square(p, a, b);
        std::complex<double> c(q.x, q.y);
        std::complex<double> z = 0;
        for (int i=0; i<n; i++) {
            z = z * z + c;
            if (std::abs(z) > 2) {
                return 1;
            }
        }
        return 0;
    }
};


struct Julia : Fractal
{
    std::complex<double> j;

    Julia(XY j, XY a, XY b, int n)
        : Fractal(a, b, n)
        , j(j.x, j.y)
    {}

    int value(XY p) const
    {
        XY q = onto_square(p, a, b);
        std::complex<double> z(q.x, q.y);
        for (int i=0; i<n; i++) {
            z = z * z + j;
            if (std::abs(z) > 2) {
                return 1;
            }
        }
        return 0;
    }
};


struct Block
{
    XY a;  // corr: be JI
    XY b;  // ^
    int a_value;
    int b_value;
    bool of_interest;
    bool env_checked;
    bool wk_interest;

    Block(XY min_, XY max_)
        : a(min_)
        , b(max_)
        , of_interest()
        , env_checked()
        , wk_interest()
    {}

    void setup(const Fractal & f, int width, int height)
    {
        XY r(width, height);
        XY c0 = onto_unit(a, xy::zero, r);
        XY c3 = onto_unit(b, xy::zero, r);
        XY c1 = XY(c3.x, c0.y);
        XY c2 = XY(c0.x, c3.y);
        int v[] = { f.value(c0), f.value(c1), f.value(c2), f.value(c3) };
        of_interest = v[0] != v[1] || v[1] != v[2] || v[2] != v[3];
        a_value = v[0];
        b_value = v[3];
    }

    bool lostcorners()
    {
        return ! of_interest && a_value == 0;
    }
};


struct Image
{
    virtual ~Image() {}

    virtual void put(int b) = 0;
};


struct Grid
{
    std::unique_ptr<Fractal> f;
    const int width;
    const int height;
    const int s;
    std::vector<int> ya;
    std::vector<int> xa;
    int n_rows;
    int n_columns;
    std::vector<Block> blocks;
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
    {
        for (int i=0; i<height; i+=s)
            ya.push_back(i);
        ya.push_back(height);
        for (int j=0; j<width; j+=s)
            xa.push_back(j);
        xa.push_back(width);
        n_rows = ya.size() - 1;
        n_columns = xa.size() - 1;

        for (int yi=0; yi<n_rows; yi++) {
            const int yl = ya[yi];
            const int yh = ya[yi + 1];
            for (int xi=0; xi<n_columns; xi++) {
                const int xl = xa[xi];
                const int xh = xa[xi + 1];
                blocks.push_back(Block(XY(xl, yl), XY(xh, yh)));
            }
        }

        for (int i=0; i<n_rows; i++) {
            for (int j=0; j<n_columns; j++) {
                Block & b = block(i, j);
                b.setup(*f.get(), width, height);
                if (b.of_interest) {
                    interest.push_back(JI(j, i));
                }
            }
        }

        for (auto e : edge) {
            XY p(e.j, e.i);
            p *= 1.0 / s;
            const int i = p.y;
            const int j = p.x;
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

    bool similar_to(const Grid & other, double similarity = 0.9965)
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

    bool points_check(int value, const std::vector<XY> & points)
    {
        const XY r(width, height);
        for (const XY & point : points) {
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
        assert (adj.i == 0 || adj.j == 0);
        if (adj.i && (i < 0 || i >= n_rows)) return false;
        if (adj.j && (j < 0 || j >= n_columns)) return false;
        Block & nbr = block(i, j);
        if (nbr.of_interest) return false;

        XY corner;
        int corner_value;
        if (adj.i + adj.j < 0) {
            corner = blk.a;
            corner_value = blk.a_value;
        } else {
            corner = blk.b;
            corner_value = blk.b_value;
        }

        std::vector<XY> r;
        if (adj.j == 0) {
            for (int x=blk.a.x; x<blk.b.x; x++) {
                r.push_back(XY(x, corner.y));
            }
        } else {
            for (int y=blk.a.y; y<blk.b.y; y++) {
                r.push_back(XY(corner.x, y));
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
        XY r(width, height);
        for (int y=0; y<height; y++) {
            const int i = y / s;
            for (int x=0; x<width; x++) {
                const int j = x / s;
                Block & blk = block(i, j);
                const int w = ! blk.of_interest ? blk.a_value
                    : f->value(onto_unit(XY(x, y), xy::zero, r));

                if (w) image.put(255);
                else image.put(0);
            }
        }
    }
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

    virtual std::unique_ptr<Fractal> fractal(int n) = 0;
    virtual std::unique_ptr<Frame> child() =  0;

    void orient(XY c, double z, int width, int height)
    {
        const double w = z * width / (double)height;
        const double h = z;
        a = c - XY(w, h) * 0.5;
        b = a + XY(w, h);
    }

    void setup(int width, int height, int n, int s)
    {
        std::unique_ptr<Grid> prev_g;
        for (;;) {
            g.reset(new Grid(fractal(n), width, height, s, {}));
            if (prev_g && prev_g->similar_to(*g.get()))
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
        g.reset(new Grid(fractal(n), g->width, g->height, g->s,
                transform(prev_edge, g->width, g->height)));
    }

    XY target()
    {
        if (fixed) return *fixed;

        c = (a + b) * 0.5;
        XY block_offset = XY(g->s, g->s) * 0.5;
        XY c_as_block_pixel = onto_square(
                onto_unit(c, a, b), xy::zero, XY(g->width, g->height))
                - block_offset;  // adjusted to just compare with block.a
        double min_pixel_distance = std::sqrt(g->width * g->width + g->height * g->height);
        XY nearest_interesting_pixel;
        bool found_interesting_pixel = false;
        for (JI kk : g->interest) {
            Block & blk = g->block(kk.i, kk.j);
            if (blk.wk_interest)
                continue;
            XY block_pixel = blk.a;
            double pixel_distance = (block_pixel - c_as_block_pixel).abs();
            if (pixel_distance < min_pixel_distance) {
                min_pixel_distance = pixel_distance;
                nearest_interesting_pixel = block_pixel;
                found_interesting_pixel = true;
            }
        }
        if ( ! found_interesting_pixel) {
            std::cerr << "Nothing of interest in frame" << std::endl;
            return c;
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

    bool lostarea(int i, int j)
    {
        return g->block(i, j).lostcorners()
            && g->block(i - 1, j).lostcorners()
            && g->block(i + 1, j).lostcorners()
            && g->block(i, j - 1).lostcorners()
            && g->block(i, j + 1).lostcorners();
    };

    bool loose(XY & lost)
    {
        if (g->interest.size() == 0)
            return false;

        int h = g->n_rows;
        int w = g->n_columns;
        std::vector<JI> ia;
        for (int i = 2; i < h - 1; i++) ia.push_back(JI(1, i));
        for (int j = 2; j < w - 1; j++) ia.push_back(JI(j, h - 2));
        for (int i = 3; i < h; i++) ia.push_back(JI(w - 2, h - i));
        for (int j = 3; j < w; j++) ia.push_back(JI(w - j, 1));
        Block * found = NULL;
        int x = 0;
        for (auto kk : ia)
        {
            Block & ib = g->block(kk.i, kk.j);
            if ( ! ib.lostcorners()) {
                found = &ib;
                break;
            }
            ++x;
        }
        if (found == NULL) return false;

        int count = 0;
        int maxcount = 0;
        Block * mostlost = NULL;
        bool inlost = false;
        for (int _ = 0; _ < ia.size(); _++) {
            if (++x == ia.size()) x = 0;
            auto kk = ia[x];
            Block & blk = g->block(kk.i, kk.j);
            if (blk.lostcorners()) {
                if (inlost) {
                    ++count;
                } else {
                    count = 1;
                    inlost = true;
                }
            } else {
                if (inlost) {
                    if (count > maxcount) {
                        int w = x - count / 2;
                        if (w < 0) w += ia.size();
                        JI m = ia[w];
                        if (lostarea(m.i, m.j)) {
                            mostlost = & g->block(m.i, m.j);
                            maxcount = count;
                        }
                    }
                    inlost = false;
                }
            }
        }
        if (mostlost == NULL) {
            int mads = 0;
            int n = 0;
            for (int _ = 0; _ < h * w / 5; _++) {
                if (n > 9)
                    break;
                int i = rnd(h - 2) + 1;
                int j = rnd(w - 2) + 1;
                if (lostarea(i, j)) {
                    ++n;
                    double dss = 0;
                    for (auto kk : g->interest) {
                        dss += std::hypot<double>(kk.i - i, kk.j - j);
                    }
                    double ads = dss / g->interest.size();
                    if (ads > mads) {
                        mads = ads;
                        mostlost = &g->block(i, j);
                    }
                }
            }
        }
        if (mostlost == NULL) {
            std::cerr << "Gave up loose" << std::endl;
            return false;
        }

        XY p = (mostlost->a + mostlost->b) * 0.5;
        lost = onto_square(onto_unit(p, xy::zero, XY(g->width, g->height)),
                a, b);
        return true;
    }
};              


struct MandelFrame : Frame
{
    MandelFrame(int width, int height, XY c, double z, int s, int n = 1)
    {
        orient(c, z, width, height);
        setup(width, height, n, s);
    }

    std::unique_ptr<Fractal> fractal(int n)
    {
        return std::unique_ptr<Fractal>(new Mandel(a, b, n));
    }

    std::unique_ptr<Frame> child()
    {
        return std::unique_ptr<Frame>(new MandelFrame(
                    g->width, g->height, target(),
                    (b.y - a.y) * 0.5, g->s, g->f->n));
    }
};


struct JuliaFrame : Frame
{
    XY j;

    JuliaFrame(XY j, int width, int height, XY c, double z, int s, int n = 1)
        : j(j)
    {
        orient(c, z, width, height);
        setup(width, height, n, s);
    }

    std::unique_ptr<Fractal> fractal(int n)
    {
        return std::unique_ptr<Fractal>(new Julia(j, a, b, n));
    }

    std::unique_ptr<Frame> child()
    {
        return std::unique_ptr<Frame>(new JuliaFrame(j,
                    g->width, g->height, target(),
                    (b.y - a.y) * 0.5, g->s, dynamic_cast<Julia *>(g->f.get())->n));
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
        std::cout << static_cast<char>(b);
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
    int width;
    int height;
    std::vector<JI> edge;
    int i;
    std::vector<int> prev_row;  // improve: operate on bool instead of int
    std::vector<int> row;
    std::vector<int> curr_row;

    EdgeDetect(int width, int height)
        : width(width), height(height), i()
    {}

    bool on_edge(int up, int left, int m, int right, int down)
    {
        return m == 0 && (up || left || right || down);
    }

    void put(int b)
    {
        curr_row.push_back(b);
        //  note: the border-pixels are not edge-detected
        //       (doesn't matter because area will be out of frame
        //       when edge is transformed to zoomed frame anyway.
        if (curr_row.size() == width) {
            if (i > 1) {
                assert(i < height);
                for (int j = 1; j < width - 1; j++) {
                    if (i % 2 == 0 && j % 2 == 0
                            && on_edge(prev_row[j], row[j-1], row[j], row[j+1], curr_row[j])) {
                        // note: take just a quarter of the points
                        //       as that's enough to cover edge.
                        edge.push_back(JI(j, i));
                    }
                }
            }
            prev_row = row;  // improve: some swap or std::move
            row = curr_row;
            curr_row.clear();
            ++i;
        }
    }
};


struct FractalMovie
{
    std::unique_ptr<Frame> frame;
    const double zi;
    const int n_loose;
    std::vector<int> counts;
    
    FractalMovie(std::unique_ptr<Frame> && _frame, int w)
        : frame(std::move(_frame))
        , zi(frame->b.y - frame->a.y)
        , n_loose(w - std::log2(frame->g->n_rows) - 1)
    {
        counts.push_back(frame->g->f->n);
        XY fixed;
        bool is_fixed = false;
        for (int i=1; i<w; i++) {
            std::unique_ptr<Frame> prev = std::move(frame);
            frame = prev->child();
            if (i >= n_loose) {
                if (prev->contained())
                    break;
                if (frame->loose(fixed)) {
                    is_fixed = true;
                }
                if (is_fixed) {
                    frame->fixed = &fixed;
                }
            }
            XY c = (frame->a + frame->b) * 0.5;
            std::cerr << i << " #" << frame->g->f->n << "("
                << c.x << ", " << c.y << ")@"
                << std::scientific << (frame->b.y - frame->a.y)
                << "\r";
            std::cerr.unsetf(std::ios_base::floatfield);
            counts.push_back(frame->g->f->n);
        }
        std::cerr << std::endl;
    }

    void generate(int k)
    {
        XY c = (frame->a + frame->b) * 0.5;
        const double ang = std::atan2(c.y, c.x);
        XY igp = XY(std::cos(ang) * 2, std::sin(ang) * (1 + frame->aspect())) * 1.1;
        //  quantity: factor is dependent of subject to get outside it ^
        XY igq = igp - c;
        const double igz = .01;
        const int u = counts.size() - 1;
        std::cerr << (k * u) << ":" << std::endl;
        std::vector<JI> edge;
        int cnt;
        for (int i=0; i<u; i++) {
            const int n = counts[i];
            const int h = counts[i + 1] - n;
            for (int j=0; j<k; j++) {
                const double d = j /(double) k;
                const double x = i + d;
                const double z = zi * std::pow(0.5, x);
                if (i < n_loose) {
                    cnt = n + static_cast<int>(h * d);
                }
                std::cerr << (i*k+j) << " #" << cnt << " @" << z << "\r";
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

    int block_side = 24;  // granularity for grid used to seach border
    int zoom_steps = 47;  // zoom to half frame width this total count
    int images_per_step = 24;

    std::srand(std::time(NULL));

    std::cerr << "Initiating focus on random location" << std::endl;
    XY mc = XY(rnd(2) - 1, rnd(2) - 1);  // mandelbrot frame center
    double mh = 2;  // mandelbrot initial frame height in complex plane
    XY jc = XY(rnd(2) - 1, rnd(2) - 1);  // julia-set ^
    double jh = 2;  // julia-set ^

    std::cerr << "Localizing Mandelbrot coordinate" << std::endl;
    FractalMovie m(std::unique_ptr<Frame>(
                new MandelFrame(width, height, mc, mh, block_side)),
            zoom_steps);
    if (do_mandel) {
        m.generate(images_per_step);
    } else {
        std::cerr << "Using location for Julia Set" << std::endl;
        FractalMovie j(std::unique_ptr<Frame>(
                    new JuliaFrame(m.frame->a, width, height, jc, jh, block_side)),
                zoom_steps);
        j.generate(images_per_step);
    }
}
