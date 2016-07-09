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

    wah_wah() : pedal(.9/SR), mix(3.1/SR) {}
    
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
    wah_wah ww;
    va_comb cb;

    void put(double v)
    {
        v = ww.shift(v);
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

    arp_wire() : c_touch() {}

    void move(double pitch)
    {
        buf.n_target = SR / f_of_just(pitch);
    }

    void touch()
    {
        c_touch += std::floor(rnd(2, 8)) * SR/4;
        buf.excite();
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
    using chord = std::array<unsigned, n_wires>;

    void draw(chord pitches)
    {
        const unsigned p = 36;
        wires[5].move(pitches[0] + p + 4);
        wires[4].move(pitches[1] + p + 9);
        wires[3].move(pitches[2] + p + 14);
        wires[2].move(pitches[3] + p + 19);
        wires[1].move(pitches[4] + p + 23);
        wires[0].move(pitches[5] + p + 28);
    }

    double get(unsigned c)
    {
        double r = 0;
        for (unsigned i=0; i<n_wires; ++i)
            r += wires[i].get(c + (1 + i)*SR/64);
        return r / n_wires;
    }
};


std::vector<arp_digitar::chord> score = {
    { 0, 0, 2, 2, 2, 0 }, //A
    { 0, 2, 2, 1, 0, 0 }, //E
    { 0, 0, 0, 2, 3, 2 }, //D

    { 0, 0, 2, 2, 1, 0 }, //Am
    { 0, 2, 2, 1, 0, 0 }, //E
    { 1, 0, 0, 2, 3, 1 }, //Dm

    { 0, 2, 2, 1, 0, 0 }, //E
    { 2, 2, 1, 2, 0, 2 }, //B7
    { 0, 0, 2, 2, 2, 0 }, //A

    { 0, 2, 2, 0, 0, 0 }, //Em
    { 2, 2, 1, 2, 0, 2 }, //B7
    { 0, 0, 2, 2, 1, 0 }, //Am

    { 0, 0, 0, 2, 3, 2 }, //D
    { 0, 0, 2, 0, 2, 0 }, //A7
    { 3, 2, 0, 0, 0, 3 }, //G

    { 1, 0, 0, 2, 3, 1 }, //Dm
    { 0, 0, 2, 2, 2, 0 }, //A
    { 3, 0, 0, 3, 3, 3 }, //Gm

    { 3, 2, 0, 0, 0, 3 }, //G
    { 0, 0, 0, 2, 1, 2 }, //D7
    { 0, 3, 2, 0, 1, 0 }, //C

    { 0, 3, 2, 0, 1, 0 }, //C
    { 3, 2, 0, 0, 0, 1 }, //G7
    { 0, 3, 3, 2, 1, 1 }, //F
};

extern "C" int isatty(int);

int main()
{
    if (isatty(1)) {
        fprintf(stderr, "redirect stdout and i'll write 1ch 16/le audio\n");
        return 1;
    }
    unsigned i = 0;
    arp_digitar aw;
    recorder rec;
    for (std::size_t c=0; c<SR*48; c++) {
        if (c % (2*SR) == 0)
            aw.draw(score[i++ % score.size()]);
        rec.put(aw.get(c));
    }
}

