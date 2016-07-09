//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "generators.hpp"
#include "math.hpp"
#include <algorithm>
#include <iostream>

namespace {

template<typename T>
void fill(generator & g, T ptr, unsigned n)
{
    const unsigned b = n / unit::size;
    for (unsigned k=0; k!=b; k++) {
        unit u;
        g.generate(u);
        std::copy(u.y, u.y + unit::size, ptr + k * unit::size);
    }
    if (unsigned r = n % unit::size) {
        unit v;
        g.generate(v);
        std::copy(v.y, v.y + r, ptr + b * unit::size);
    }
}

inline void mod1inc(double & x, double d)
{
    if (fabs(d) >= 1) {
        d = fmod(d, 1);
        std::cerr << "mod jump" << std::endl;
    }
    x += d;
    if (x >= 1) x -= 1;
    else if (x < 0) x += 1;
}

inline double famp(double z) { return z * z; }

void prune(std::vector<weighted_transform::component> & s)
{
    s.erase(std::remove_if(s.begin(), s.end(),
                [](const weighted_transform::component & w){
                return ! w.e->more();
                }),
            s.end());
}

}

generator::~generator() {}

bool infinite::more() { return true; }

void silence::generate(unit & u) { u.set(0); }

noise::noise(double amplitude) : amplitude_(amplitude) {}

void noise::generate(unit & u)
{
    const double r = amplitude_;
    std::generate(std::begin(u.y), std::end(u.y),
            [r](){ return rnd(-r, r); });
}

gen::gen(en_ptr e) : e(e), units_generated_() {}

namespace {
double t_generated(unsigned u) { return u * (unit::size /(double) SR); }
}

void gen::generate(unit & u)
{
    using namespace std::placeholders;
    auto f = std::bind(&envelope::y, e.get(), _1);
    double x = t_generated(units_generated_++);
    std::generate(std::begin(u.y), std::end(u.y),
            [&x, f](){ return f(x += 1./SR); });
}

bool gent::more() { return s > t_generated(units_generated_); }

gent::gent(en_ptr e, double s) : gen(e), s(s) {}

genv::genv(ug_ptr && g, double s, en_ptr e, bool hang)
        : g(std::move(g)), e(e), hang(hang), s(s), units_generated_()
{}

void genv::generate(unit & u)
{
    double t = t_generated(units_generated_++);
    if (t > s) {
        if (hang) g->generate(u);
        else u.set(0);
    } else {
        g->generate(u);
        for (auto & x : u.y)
            x *= e->y(t += 1./SR);
    }
}

bool genv::more()
{
    return hang ||
        (g->more() && s > t_generated(units_generated_));
}

filtration::filtration(ug_ptr && g, fl_ptr l, double linger_t)
    : g(std::move(g)), l(l), linger_units(std::ceil(SR * linger_t / unit::size))
{}

void filtration::generate(unit & u)
{
    g->generate(u);
    for (auto & x : u.y) x = l->shift(x);
}

bool filtration::more()
{
    if (g->more()) return true;
    if (linger_units == 0) return false;
    --linger_units;
    return true;
}

ncopy::ncopy(int n, ug_ptr && g) : i(), n(n), g(std::move(g)) {}

void ncopy::generate(unit & u)
{
    if (i == 0) g->generate(v);

    u = v;

    if (++i >= n) i = 0;
}

bool ncopy::more() { return g->more(); }

wrapshared::wrapshared(std::shared_ptr<generator> g) : g(g) {}
void wrapshared::generate(unit & u) { g->generate(u); }
bool wrapshared::more() { return g->more(); }

bool record::more() { return samples_generated_ < buffer_.w.size(); }

unsigned record::size() { return buffer_.w.size(); }

void record::reset()
{
    t_reset += samples_generated_ /(double) SR;
    buffer_.cycle(duration_->y(t_reset) * SR);
    samples_generated_ = 0;
}

record::record(generator & g, en_ptr duration)
    : buffer_(duration->y(0) * SR)
    , duration_(duration)
    , samples_generated_()
    , t_reset()
{
    ::fill(g, buffer_.w.begin(), buffer_.w.size());
}

void record::generate(unit & u)
{
    if (samples_generated_ + unit::size < buffer_.w.size()) {
        std::copy(buffer_.w.begin(), buffer_.w.begin() + unit::size,
                std::begin(u.y));
        samples_generated_ += unit::size;
    } else if (samples_generated_ < buffer_.w.size()) {
        const unsigned h = buffer_.w.size() - samples_generated_;
        std::copy(buffer_.w.begin(), buffer_.w.begin() + h, std::begin(u.y));
        std::fill(std::begin(u.y) + h, std::end(u.y), 0);
        samples_generated_ = buffer_.w.size();
    } else {
        u.set(0);
    }
}

periodic::carry_buffer::carry_buffer() : head(), tail(), a() {}

unsigned periodic::carry_buffer::n()
{
    if (head == tail) return 0;
    else if (head > tail) return head - tail;
    else return head + (unit::size * 2 - tail);
}

unsigned periodic::carry_buffer::post_incr(unsigned & i)
{
    const unsigned r = i;
    if (++i >= unit::size * 2) i = 0;
    return r;
}

void periodic::carry_buffer::put(double y) { a[post_incr(head)] = y; }

double periodic::carry_buffer::get() { return a[post_incr(tail)]; }

void periodic::append(unit & u, unsigned n)
{
    carry_buffer * p = carry_.get();
    std::for_each(std::begin(u.y), std::begin(u.y) + n,
            [p](double v){ p->put(v); });
}

void periodic::shift(unit & u)
{
    if (carry_->n() < unit::size) throw std::runtime_error("short carry");

    carry_buffer * p = carry_.get();
    std::generate(std::begin(u.y), std::end(u.y),
            [p](){ return p->get(); });
}

periodic::periodic(pg_ptr && g)
    : g_(std::move(g))
    , carry_(new carry_buffer)
{}

void periodic::generate(unit & u)
{
    for (;;) {
        unsigned n = unit::size;
        unit v;
        g_->generate(v);
        if ( ! g_->more()) {
            if (unsigned r = g_->size() % unit::size) n = r;
            g_->reset();
        }
        append(v, n);
        if (carry_->n() >= unit::size) break;
    }
    shift(u);
}

karpluss::karpluss(fl_ptr l, generator & g, en_ptr duration)
    : record(g, duration), filter_(l)
{}

void karpluss::reset()
{
    record::reset();
    for (double & x : buffer_.w) x = filter_->shift(x);
}

multiply::multiply(ug_ptr && a, ug_ptr && b)
    : a_(std::move(a)), b_(std::move(b))
{}

void multiply::generate(unit & u)
{
    unit v;
    a_->generate(v);
    b_->generate(u);
    std::transform(std::begin(v.y), std::end(v.y),
            std::begin(u.y),
            std::begin(u.y), std::multiplies<double>());
}

bool multiply::more() { return a_->more() && b_->more(); }

weighted_transform::weighted_transform(double i) : init_(i), anymore_() {}

void weighted_transform::c(ug_ptr && g, double w)
{
    s_.emplace_back(std::move(g), w);
    anymore_ = true;
}

void weighted_transform::generate(unit & u)
{
    anymore_ = false;
    u.set(init_);
    for (unsigned i=0; i!=s_.size(); i++) {
        if (s_[i].e->more()) {
            unit v;
            s_[i].e->generate(v);
            f(u, v, s_[i].w);
            if (s_[i].e->more()) anymore_ = true;
        }
    }
    prune(s_);
}

bool weighted_transform::more() { return anymore_; }

sum::sum() : weighted_transform(0) {}

void sum::f(unit & accumulator, unit & u, double w)
{
    accumulator.add(u, w);
}

product::product() : weighted_transform(1) {}

void product::f(unit & accumulator, unit & u, double w)
{
    accumulator.mul(u, w);
}

modulation::modulation(ug_ptr && modulator, en_ptr carrier, en_ptr freq)
    : modulator_(std::move(modulator))
    , carrier_(carrier)
    , freq_(freq)
    , x_()
    , t_generated_()
{}

void modulation::generate(unit & u)
{
    unit v;
    modulator_->generate(v);

    constexpr double t_per_sample = 1./ SR;
    for (unsigned i=0; i<unit::size; ++i) {
        u.y[i] = carrier_->y(x_);
        mod1inc(x_, t_per_sample * (1 + v.y[i]) * freq_->y(t_generated_));
        t_generated_ += t_per_sample;
    }
}

delayed_sum::term::term(double t, ug_ptr && g)
    : t(t), g(std::move(g)), offset(unsigned(t * SR) % unit::size)
{}

delayed_sum::glue_buffer::glue_buffer() : c(2) { a.set(0); b.set(0); }
void delayed_sum::glue_buffer::add(unsigned h, unit & u)
{
    if (h > unit::size) throw std::runtime_error("offsets beyond");
    const unsigned r = unit::size - h;

    std::transform(std::begin(u.y), std::begin(u.y) + r,
            std::begin(a.y) + h,
            std::begin(a.y) + h, std::plus<double>());

    std::transform(std::begin(u.y) + r, std::end(u.y),
            std::begin(b.y),
            std::begin(b.y), std::plus<double>());
    c = 0;
}
void delayed_sum::glue_buffer::flush(unit & u)
{
    u = a;
    a = b;
    b.set(0);
    ++c;
}
bool delayed_sum::glue_buffer::more_to_flush() { return c < 2; }

delayed_sum::delayed_sum()
    : buffer_(new glue_buffer)
    , pending_()
    , units_generated_()
{}

void delayed_sum::c(ug_ptr && g, double t)
{
    if (terms_.size() && terms_.back().t > t)
        throw std::runtime_error("terms out of chronology");
    terms_.emplace_back(t, std::move(g));
    pending_ = true;
}

void delayed_sum::generate(unit & u)
{
    pending_ = false;
    const double s = ++units_generated_ * unit::size /(double) SR;
    unsigned z = 0;
    bool h = false;
    for (auto & e : terms_) {
        if (e.g->more()) {
            if (e.t >= s) {
                pending_ = true;
                break;
            }
            unit w;
            e.g->generate(w);
            buffer_->add(e.offset, w);
            h = true;
        } else if ( ! h) ++z;
    }
    terms_.erase(terms_.begin(), terms_.begin() + z);
    buffer_->flush(u);
}

bool delayed_sum::more() { return pending_ || buffer_->more_to_flush(); }

lazy::lazy(bs_ptr b) : builder_(b) {}

void lazy::generate(unit & u)
{
    generator_->generate(u);
}

bool lazy::more()
{
    if ( ! generator_) {
        generator_ = builder_->build();
        builder_ = nullptr;
    }
    return generator_->more();
}

void limiter::out(unit & u, double target)
{
    const double d = (target - glider_) / unit::size;
    const double a = glider_;
    int i = 0; // quality: glide stepping on brownian stairs?
    std::transform(std::begin(buffer_unit_.y), std::end(buffer_unit_.y),
            std::begin(u.y),
            [a, d, &i](double x)
            {
            double y = x * (a + d * i++);
            if (std::abs(y) > 1) { // puzzled: seems to happen
            y = std::copysign(1, y);
            //throw std::runtime_error("limiter malfunction");
            }
            return y;
            });
}

limiter::limiter(ug_ptr && z)
    : generator_(std::move(z))
    , started_()
    , quit_()
    , increment_(.001)
    , s_(1)
    , glider_(1)
{}

void limiter::generate(unit & u)
{
    if ( ! started_) {
        started_ = true;
        generator_->generate(buffer_unit_);
        // improve: i guess s_ and glider_ should be set here instead
    }
    unit w;
    generator_->generate(w);

    // optimization: prolly more efficient to do max_element([]abs e)
    const auto p = std::minmax_element(std::begin(w.y), std::end(w.y));
    const double a = std::max(std::abs(*p.first), std::abs(*p.second));
    const double m = (a > 1) ? 1/a : 1;
    double target = s_;
    if (target > m) target = m;
    if (target > glider_ + increment_) target = glider_ + increment_;
    out(u, target);
    glider_ = target;
    buffer_unit_ = w;
    s_ = m;
}

bool limiter::more()
{
    if (quit_) return false;
    if (generator_->more()) return true;

    quit_ = true;
    return true;
}

inter::inter(ug_ptr && l, ug_ptr && r)
    : l(std::move(l)), r(std::move(r)), n(), j()
{}

void inter::generate(unit & u)
{
    if ((n & 1) == 0) {
        l->generate(lu);
        r->generate(ru);
        j = 0;
    }
    for (unsigned i=0; i!=unit::size; i++)
    {
        const unsigned h = j >> 1;
        u.y[i] = (j++&1)==0 ? lu.y[h] : ru.y[h];
    }
    n++;
}

bool inter::more() { return (n & 1) != 0 || l->more() || r->more(); }
