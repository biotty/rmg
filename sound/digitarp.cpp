//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "musicm.hpp"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <array>

#define SR 44100

const double pi = 3.141592653589;
double rnd(double a, double b) { return a + rand() * (b - a) / RAND_MAX; }
double sine(double x) { return sin(x * 2 * pi); }
double linear(double a, double b, double r) { return a * (1 - r) + b * r; }

struct lfo
{
    double d, x;

    lfo(double d = 0) : d(d), x(rnd(0, 1)) {}
    
    double y()
    {
        if ((x += d) > 1) x -= 1;
        return sine(x);
    }

    double linear(double a, double b)
    {
        return ::linear(a, b, .5 * (1 + y()));
    }
};

struct ring_buffer
{
    std::vector<float> samples;
    unsigned i;
    
    ring_buffer(unsigned n) : samples(n), i()
    {}
    
    void ring_walk()
    {
        if (i == 0) i = samples.size();
        --i;
    }

    float & element(unsigned k = 0)
    {
        return samples[(i + k) % samples.size()];
    }
};

struct worm_buffer : ring_buffer
{
    lfo delay;
    unsigned n;
    unsigned n_target;
    double step_progress;
    double step_speed;

    worm_buffer(unsigned n = 0)
        : ring_buffer(SR/10)
        , n(n)
        , n_target(n)
        , step_progress()
        , step_speed(.01)
    {}

    void grow()
    {
        if (n == 0) {
            n = n_target;
            return;
        }
        
        if (n_target != n) {
            step_progress += step_speed;
            if (step_progress >= 1) {
                step_progress = 0;
                if (n_target > n) n += 1;
                else n -= 1;
            }
        }
    }

};

struct va_karpluss : worm_buffer
{
    void excite()
    {
        const double y = rnd(.8, 1);
        for (size_t k=1; k<n; k++)
            element(k) += rnd(-y, y);
    }

    double shift()
    {
        grow();

        const unsigned k = n * linear(.99, .999, .5 * (1 + delay.y()));
        const double r = .99 * linear(element(k - 1), element(k), .8);
        element() = r;
        ring_walk();
        return r;
    }
};

struct va_comb : worm_buffer
{
    lfo fundamental;
    va_comb() : fundamental(.4/SR) {}
    double shift(double v) {
        n_target = SR/fundamental.linear(48, 880);
        grow();

        const double r = linear(v, element(n), .3);
        element() = v;
        ring_walk();
        return r;
    }
};

class biquad
{
    double w1, w2;
public:
    double a1, a2, b0, b1, b2;
    
    biquad() : w1(), w2(), a1(), a2(), b0(), b1(), b2() {}
    
    double shift(double x)
    {
        const double w0 = x - a1 * w1 - a2 * w2;
        const double y = b0 * w0 + b1 * w1 + b2 * w2;
        w2 = w1;
        w1 = w0;
        return y;
    }

    void set_wah(double c, double q)
    {
      const double p = 2 * pi * c / SR;
      const double alpha = sin(p) / (2 * q);
      const double a0 = 1 + alpha;
      a1 = (-2 * cos(p)) / a0;
      a2 = (1 - alpha) / a0;
      b0 = alpha / a0;
      b2 = -b0;
    }
};

struct wah_wah
{
    biquad f;
    lfo pedal;
    lfo mix;

    wah_wah() : pedal(9/SR), mix(31/SR) {}
    
    double shift(double v)
    {
        double c = pedal.linear(36, 880);
        f.set_wah(c, .98);
        return linear(v, f.shift(v),
                mix.linear(.5, .6));
    }
};

struct echo
{
    double a;
    lfo delay;
    unsigned cached_d;
    echo(double a) : a(a), cached_d() {}
};

struct reverbr : ring_buffer
{
    std::vector<echo> echoes;
    reverbr() : ring_buffer(SR)
    {
        for (unsigned k=0; k<9; k++) {
            echo e(rnd(.01, .03));
            e.delay.d = rnd(.1, 1)/SR;
            echoes.push_back(e);
        }
    }

    void move_echoes()
    {
        for (unsigned k=0; k<echoes.size(); k++) {
            echo & e = echoes[k];
            e.cached_d = linear(1, samples.size(),
                        .5 * (1 + e.delay.y()));
        }
    }

    double get()
    {
        static unsigned c = 0;
        if (c) --c;
        else {
            c = 441;
            move_echoes();
        }
        double r = 0;
        for (unsigned k=0; k<echoes.size(); k++) {
            const echo & e = echoes[k];
            r += e.a * element(e.cached_d);
        }
        return r;
    }

    void put(double v)
    {
        element() = v;
    }
};

struct recorder
{
    reverbr re;
    wah_wah wa;
    wah_wah wb;
    va_comb cb;

    void put(double v)
    {
        v = wa.shift(v);
        v = wb.shift(v);
        v = cb.shift(v);
        v += re.get();
        re.put(v);
        
        if (v > 1) v = 1;
        else if (v < -1) v = -1;
        int16_t b = v * 32767;
        // on-need: swap bytes if be arch (detect in constructor)
        fwrite(&b, 2, 1, stdout);
    }
};

struct arp_wire
{
    va_karpluss buf;
    unsigned c_touch;
    bool enabled;

    arp_wire() : c_touch(), enabled() {}

    void move(double pitch)
    {
        buf.n_target = SR / f_of_just(pitch);
    }

    void touch()
    {
        c_touch += std::floor(rnd(1, 4)) * SR/8;
        if (enabled) buf.excite();
    }

    double get(unsigned c)
    {
        if (c > c_touch) touch();
        return buf.shift();
    }
};

struct arp_digitar
{
    static const unsigned n_wires = 6;
    arp_wire wires[n_wires];
    using chord = std::array<int, n_wires>;

    void draw(chord pitches)
    {
        constexpr unsigned p = 36;
        constexpr unsigned a[] = {
            p + 4, p + 9,
            p + 14, p + 19,
            p + 23, p + 28 };

        for (unsigned i=0; i<n_wires; i++) {
            arp_wire & w = wires[n_wires - 1 - i];
            if (pitches[i] >= 0) {
                w.enabled = true;
                w.move(a[i] + pitches[i]);
            } else {
                w.enabled = false;
                w.move(a[i]);
            }
        }
    }

    double get(unsigned c)
    {
        double r = 0;
        for (unsigned i=0; i<n_wires; ++i)
            r += wires[i].get(c);
        return r / n_wires;
    }
};

constexpr int X = -1;

std::vector<arp_digitar::chord> score = {
    { X, X, X, X, X, X },

    { X, 0, 2, 2, 2, 0 }, //A
    { 0, 2, 2, 1, 0, 0 }, //E
    { X, X, 0, 2, 3, 2 }, //D

    { X, 0, 2, 2, 1, 0 }, //Am
    { 0, 2, 2, 1, 0, 0 }, //E
    { X, X, 0, 2, 3, 1 }, //Dm

    { 0, 2, 2, 1, 0, 0 }, //E
    { X, 2, 1, 2, 0, 2 }, //B7
    { X, 0, 2, 2, 2, 0 }, //A

    { 0, 2, 2, 0, 0, 0 }, //Em
    { X, 2, 1, 2, 0, 2 }, //B7
    { X, 0, 2, 2, 1, 0 }, //Am

    { X, X, 0, 2, 3, 2 }, //D
    { X, 0, 2, 0, 2, 0 }, //A7
    { 3, 2, 0, 0, 0, 3 }, //G

    { X, X, 0, 2, 3, 1 }, //Dm
    { X, 0, 2, 2, 2, 0 }, //A
    { 3, 0, 0, 3, 3, 3 }, //Gm

    { 3, 2, 0, 0, 0, 3 }, //G
    { X, X, 0, 2, 1, 2 }, //D7
    { X, 3, 2, 0, 1, 0 }, //C

    { X, 3, 2, 0, 1, 0 }, //C
    { 3, 2, 0, 0, 0, 1 }, //G7
    { X, X, 3, 2, 1, 1 }, //F

    { X, X, X, X, X, X },
    { X, X, X, X, X, X },
};

extern "C" int isatty(int);

int main()
{
    if (isatty(1)) {
        fprintf(stderr, "redirect stdout and i'll write 1ch 16/le audio\n");
        return 1;
    }
    arp_digitar aw;
    recorder rec;
    unsigned i = 0;
    unsigned c = 0;
    constexpr unsigned r = SR * 4;
    while (true) {
        if (c % r == 0) {
            if (i == score.size()) break;
            aw.draw(score[i++]);
        }
        rec.put(aw.get(c++));
    }
}

