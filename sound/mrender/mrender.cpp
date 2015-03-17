//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "midi.hpp"
#include "biquad.hpp"
#include "samplerate.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cmath>
#include <cassert>
#include <vector>
#include <list>
#include <set>
#include <memory>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iomanip>

#include <unistd.h>

double const lim_amp = .8;
double const amp_per_sec = 1;
double const ear_secs = .0007;
double const dist_amp = .6;
double const pi = 3.1415926535;
double sign(double r) { return r >= 0 ? 1 : -1; }
double linear(double a, double b, double r) { return a * (1 - r) + b * r; }
double rnd(double a, double b) { return a + rand() * (b - a) / RAND_MAX; }
double frequency_of(double p) { return 440 * pow(2, (p - 69) / 12); }
double pitch_of(double f) { return 69 + 12 * log2(f / 440); }
double amplitude_of(double l) { return pow(10, l / 20); }
double loudness_of(double a) { return 20 * log10(a); }

double sine(double x) { return sin(x * 2 * pi); }

double square(double x)
{
    x = fmod(x, 1);
    if (x < 0) x += 1;
    return sign(x - .5);
}

double sawtooth(double x)
{
    x = fmod(x, 1);
    if (x < 0) x += 1;
    const double r = x * 4;
    if (r > 3) return r - 4;
    if (r > 1) return 2 - r;
    return r;
}

double squareshape(double y, double p)
{
    if (y >= 0) return pow(y, p);
    return -pow(-y, p);
}

double sawtoothpull(double x, double p)
{
    x = fmod(x, 1);
    if (x < 0) x += 1;
    x = (squareshape(2 * (x - .5), p) + 1) * .5;
    return x;
}

struct wavefun { virtual double operator()(double x) = 0; };
struct whitenoisefun : wavefun { double operator()(double) { return rnd(-1, 1); } };
struct sinefun : wavefun { double operator()(double x) { return sine(x); } };
struct squarefun : wavefun { double operator()(double x) { return square(x); } };
struct sawtoothfun : wavefun { double operator()(double x) { return sawtooth(x); } };

struct qshapefun : wavefun
{
    double q;
    qshapefun(double a) : q(pow(10, -a)) {}
    double operator()(double x) { return squareshape(sine(x), q); }
};

struct wshapefun : wavefun
{
    double w;
    wshapefun(double a) : w(pow(10, -a)) {}
    double operator()(double x) { return sine(sawtoothpull(x, w)); }
};

struct qwshapefun : wavefun
{
    double q, w;
    qwshapefun(double a, double b)
        : q(pow(10, -a)), w(pow(10, -b))
    {}
    double operator()(double x) {
        return squareshape(sine(sawtoothpull(x, w)), q);
    }
};

typedef std::vector<float> samples;

double enveloping_amplitude(samples::iterator b, samples::iterator e)
{
    double r = 0;
    for (samples::iterator it = b; it != e; ++it) {
        const float a = fabsf(*it);
        if (a > r) r = a;
    }
    return r;
}

struct sampler
{
    samples c;
    double i, s, g;
    sampler(double i = amp_per_sec/10) : i(i), s(), g() {}
    void write(samples w)
    {
        double a = enveloping_amplitude(w.begin(), w.end());
        const double h = lim_amp;
        const double m = (a >= h) ? h/a : h;
        if ( ! c.empty()) {
            double t = s;
            if (t > m) t = m;
            if (t > g+i) t = g+i;
            flush(t);
        } else
            g = m;
        s = m;
        c = w;
    }
    void flush(double t)
    {
        const double m = 1.0 / c.size();
        double a = g;
        for (unsigned j=0; j<c.size(); j++) {
            const double v = c[j];
            if ((j & 1) == 0 && a != t)
                a = linear(g, t, j * m);
            put(v * a);
        }
        g = t;
    }
    void close()
    {
        if (c.empty()) return;
        flush(0);
        c.resize(0);
    }
    void put(double y)
    {
        if (y < -1) y = -1;
        else if (y > 1) y = 1;
        int i = y * 32767;
        signed char cd[2];
        cd[0] = i >> 8;
        cd[1] = i & 255;
        fwrite(cd, 2, 1, stdout);
    }
};

typedef std::shared_ptr<wavefun> wavefunptr;

wavefunptr defaultfun() { return wavefunptr(new sinefun); }

struct vibrato {
    double a;
    double f;
    wavefunptr s;

    vibrato() : a(), f(1), s(defaultfun()) {}
    vibrato(double p, double e, double f, wavefunptr s = defaultfun()) : f(f), s(s)
    {
        const double q = frequency_of(p);
        a = (frequency_of(p + e) - q) / q;
    }
    double get(double x)
    {
        return 1 + a * (*s)(x * f);
    }
};

struct tremolo {
    double k;
    double f;
    wavefunptr s;

    tremolo() : k(), f(1), s(defaultfun()) {}
    tremolo(double l, double f, wavefunptr s = defaultfun())
        : k((1 - amplitude_of(l)) / 2), f(f), s(s)
    {}
    double get(double x)
    {
        return 1 + k * ((*s)(x * f) - 1);
    }
};

class envelope {
    double a, d, s, r, k, u, q;
    double f(double x)
    {
        return (x < a) ? x / a
                : (x < a + d)
                    ? linear(1, s, (x - a) / d)
                    : s;
    }
public:
    envelope(double a, double d, double s, double u, double r)
        : a(a), d(d), s(s), r(r), k(1 / r), u(u), q(f(u)) {}
    double get(double x)
    {
        assert(x >= 0);
        if (x <= u) return f(x);
        const double z = x - u;
        if (z >= r) return 0;

        return linear(q, 0, z * k);
    }
    double span()
    {
        return u + r;
    }
};

struct sourcewave {
    virtual double get(double d) = 0;
    sourcewave() : x() {}
    virtual void start() {}
protected:
    void advance(double d)
    {
        x += d;
        if (x >= 1) { x -= 1; start(); }
        else if (x < 0) x += 1;
    }
    double x;
};

struct whitenoisewave : sourcewave
{
    double get(double)
    {
        return rnd(-1, 1);
    }
};

struct bandgrainwave : sourcewave
{
    double b;
    double e;
    bandgrainwave(double b) : b(b), e() { start(); x = rnd(0, 1); }
    void start() { e = rnd(1, 1 + b); }
    double get(double d)
    {
        double r = sine(x);
        advance(d * e);
        return r;
    }
};

struct bandnoisewave : sourcewave
{
    std::vector<bandgrainwave> w;
    bandnoisewave(unsigned n, double b) : w(n, b) {}
    double get(double d)
    {
        double r = 0;
        for (unsigned i=0; i<w.size(); i++) r += w[i].get(d);
        return r / w.size();
    }
};

struct modulatorwave : private sourcewave {
    double t, a;
    wavefunptr s;
    modulatorwave(double t, double a, wavefunptr s = defaultfun()) : t(t), a(a), s(s) {}
    double get(double d)
    {
        const double r = a * (*s)(x);
        advance(d * t);
        return r;
    }
};

struct fmwave : sourcewave {
    wavefunptr s;
    modulatorwave m;
    fmwave(double r, double f, wavefunptr s = defaultfun())
        : s(s), m(f, r)
    {}
    double get(double d)
    {
        const double r = (*s)(x);
        advance(d * (1 + m.get(d)));
        return r;
    }
};

struct stringwave : sourcewave {
    filter::biquad f;
    wavefunptr s;
    samples a;
    unsigned i;
    stringwave(double p, wavefunptr s, filter::biquad f)
        : f(f), s(s), a(SAMPLERATE / frequency_of(p)), i()
    {
        const unsigned n = a.size();
        for (unsigned i=0; i<n; i++)
            a[i] = (*s)(i /(double) n);
    }

    double get(double /*d*/)
    {
        const double r = a[i] = f.shift(a[i]);
        if (++i >= a.size()) i = 0;
        return r;
    }
};

struct participantwave : private sourcewave {
    double t;
    wavefunptr s;
    participantwave(double t, wavefunptr s = defaultfun()) : t(t), s(s)
    { x = rnd(0, 1); }
    double get(double d)
    {
        const double r = (*s)(x);
        advance(d * t);
        return r;
    }
};

struct choruswave : sourcewave {
    std::vector<participantwave> a;
    choruswave(unsigned n, double t = .01)
    { for (unsigned i=0; i<n; i++) a.push_back(participantwave(1 + t)); }
    double get(double d)
    {
        double r = 0;
        for (unsigned i=0; i<a.size(); i++) r += a[i].get(d);
        return r / (a.size());
    }
};

struct formant {
    formant(double x, double y, double w, double a, double r)
        : p(x - w), q(x + w), y(y), a(a), r(r)
    {}
    double get(double x)
    {
        if (p - x >= a || x - q >= r)  return 0;
        if (x <= p) return linear(0, y, (a - (p - x)) / a);
        if (x >= q) return linear(y, 0, (r - (x - q)) / r);
        return y;
    }
    double z() { return q + r; }
private:
    double p, q, y, a, r;
};

class harmonicwave : public sourcewave {
    std::vector<double> a;
public:
    bool p;
    harmonicwave(std::vector<formant> o, bool p = false)
        : p(p)
    {
        unsigned n = 0;
        for (unsigned c=0; c<o.size(); c++) {
            const double z = o[c].z();
            if (n < z) n = z;
        }
        a.resize(n);
        for (unsigned q=0; q<n; q++) {
            double s = 0;
            for (unsigned i=0; i<o.size(); i++) {
                double y = o[i].get(q);
                if (s < y) s = y;
            }
            a[q] = s;
        }
    }
    double get(double d)
    {
        double r = 0;
        const unsigned i = p ? 2 : 1;
        for (unsigned q=0; q<a.size(); q+=i)
            r += a[q] * sine(x * (q + 1));
        advance(d);
        return r / a.size();
    }
};

typedef std::shared_ptr<sourcewave> waveptr;

typedef std::pair<double, double> stereo;

class sound {
    bool fromright;
    unsigned n;
    double d, a, m;
    waveptr w;
    vibrato v;
    tremolo r;
    envelope e;
    filter::biquad f;
    filter::delay g;
public:
    double t;
    double z;
    sound(double t, double p, double l, double o, waveptr w,
            vibrato v, tremolo r, envelope e)
        : d(frequency_of(p) / SAMPLERATE), a(amplitude_of(l))
        , m(linear(dist_amp, 1, fabs(sin(o))))
        , w(w), v(v), r(r), e(e), t(t)
    {
        double c = cos(o);
        fromright = c > 0;
        double fardelay = fabs(c) * ear_secs;
        z = t + e.span() + fardelay;
        n = fardelay * SAMPLERATE;

        double frombehind = .5 * (1 - sin(o));
        f.lowpass(linear(8000, 1000, frombehind));
    }
    void alloc() { g.alloc(n); }
    stereo get(double s)
    {
        if (s < t || s > z) return stereo(0, 0);
        s -= t;
        const double y = a * r.get(s) * w->get(d * v.get(s));
        const double near = e.get(s) * f.shift(y);
        const double far = g.shift(near) * m;
        return fromright ? stereo(far, near) : stereo(near, far);
    }
    bool operator<(const sound & other) const { return t < other.t; }
};

struct instrument
{
    virtual void play(midi::note n, std::multiset<sound> & r) = 0;
};

struct hihat : instrument
{
    void play(midi::note n, std::multiset<sound> & p)
    {
        vibrato v;
        tremolo r;
        envelope en = envelope(0, .01, .5, .01, .04);
        whitenoisewave * y = new whitenoisewave();
        p.insert(sound(n.t, 33, n.l - 6, n.o, waveptr(y), v, r, en));
        //envelope ec = envelope(0.03, .03, .4, 0.06, .12);
        //choruswave * w = new choruswave(5, 1);
        //p.insert(sound(n.t, 93, n.l - 9, n.o, waveptr(w), v, r, ec));
    }
};

struct drum : instrument
{
    void play(midi::note n, std::multiset<sound> & p)
    {
        vibrato v;
        tremolo r;
        envelope ew = envelope(0, 0, 1, 0.03, 0.2);
        envelope ey = envelope(0.04, 0, 1, 0.04, 0.1);
        bandnoisewave * w = new bandnoisewave(19, 3);
        choruswave * y = new choruswave(9, 2);
        y->a[0].s = wavefunptr(new sawtoothfun);
        y->a[1].s = wavefunptr(new squarefun);
        p.insert(sound(n.t, 33, n.l, n.o, waveptr(w), v, r, ew));
        p.insert(sound(n.t, 21, n.l, n.o, waveptr(y), v, r, ey));
    }
};

class synth : public instrument
{
    wavefunptr w() { return wavefunptr(new qwshapefun(rnd(-.5, .5), rnd(-.5, .5))); }
public:
    void play(midi::note n, std::multiset<sound> & p)
    {
        vibrato v = vibrato(n.p, rnd(0, .1), rnd(1, 16), w());
        tremolo r = tremolo(rnd(0, -6), rnd(1, 16), w());
        envelope es = envelope(rnd(0, .03), rnd(.05, .1), rnd(.5, .7), n.d, rnd(.03, .1));
        envelope en = envelope(rnd(0, .2), rnd(.1, .2), rnd(.7, 1), n.d, rnd(.1, .5));
        double f = 1;
        switch (unsigned(rnd(0, 4))) {
        case 0: f *= .5; break;
        case 1: f *= 1.5; break;
        case 2: f *= 2; break;
        case 3: f *= 3; break;
        }
        f *= rnd(0.98, 1.02);
        fmwave * y = new fmwave(rnd(1.7, 7.9), f, w());
        bandnoisewave * z = new bandnoisewave(4, .1);
        p.insert(sound(n.t, n.p, n.l - 18, n.o, waveptr(y), v, r, es));
        p.insert(sound(n.t, n.p, n.l - 12, n.o, waveptr(z), v, r, en));
    }
};

class vowel : public instrument
{
public:
    void play(midi::note n, std::multiset<sound> & p)
    {
        vibrato v(n.p, rnd(0.05, .1), rnd(3, 8));
        tremolo r(-2, rnd(3, 8));
        envelope ex = envelope(0, 0, 1, n.d, rnd(.1, .2));
        envelope ey = envelope(0, 0, 1, n.d, rnd(.1, .2));
        std::vector<formant> f;
        f.push_back(formant(rnd(0, 4.5), rnd(.5, .9), rnd(0, 2), rnd(3, 6), rnd(3, 6)));
        f.push_back(formant(rnd(6, 9.5), rnd(.5, .7), rnd(0, 2), rnd(4, 7), rnd(4, 7)));
        f.push_back(formant(rnd(12, 15.5), rnd(.1, .5), rnd(0, 2), rnd(5, 8), rnd(5, 8)));
        harmonicwave * x = new harmonicwave(f, true);
        harmonicwave * y = new harmonicwave(f, true);
        p.insert(sound(n.t, n.p, n.l, n.o, waveptr(x), v, r, ex));
        p.insert(sound(n.t + rnd(0.04, 0.1), n.p, n.l, n.o, waveptr(y), v, r, ey));
    }
};

class wind : public instrument
{
public:
    void play(midi::note n, std::multiset<sound> & p)
    {
        vibrato v;
        tremolo r;
        envelope ex = envelope(0.02, 0, 1, 0.02, 0.3);
        envelope ey = envelope(0.1, 0, 1, 0.1, 0.02);
        bandnoisewave * x = new bandnoisewave(19, 5);
        bandnoisewave * y = new bandnoisewave(19, 5);
        p.insert(sound(n.t, rnd(70, 117), n.l - 3, n.o, waveptr(x), v, r, ex));
        p.insert(sound(n.t, rnd(70, 117), n.l - 6, n.o, waveptr(y), v, r, ey));
    }
};

class string : public instrument
{
    wavefunptr w() { return wavefunptr(new whitenoisefun); }
public:
    void play(midi::note n, std::multiset<sound> & p)
    {
        vibrato v;
        tremolo r;
        envelope e = envelope(0, 0, 1, n.d, 0.6);
        filter::biquad f;
        f.lowpass(4 * frequency_of(n.p));
        stringwave * x = new stringwave(n.p, w(), f);
        p.insert(sound(n.t, 0, n.l, n.o, waveptr(x), v, r, e));
    }
};

struct orchestra
{
    synth s;
    vowel v;
    hihat h;
    drum d;
    wind w;
    string g;
    instrument * get_program(midi::codepoint i) {
        if (i.is_percussion)
            switch (i.n) {    
            case 35: case 36: case 51: return &d;
            case 39: case 44: return &w;
            default: return &h;
            }
        if ((i.n % 3) == 0) return &g;
        else if ((i.n % 3) == 1) return &s;
        else return &v;
    }
};

class conductor // destructive -- eats the set of targeted sound
{
    sampler o;
    std::multiset<sound> & s;
    void trim(double t, std::list<sound> & r)
    {
        while ( ! s.empty() && s.begin()->t < t) {
            const std::multiset<sound>::iterator s_first = s.begin();
            r.push_front(*s_first);
            s.erase(s_first);
            r.front().alloc();
        }
    }
    void chunk(double t, std::list<sound> & p, samples & a)
    {
        const double d = 1.0 / SAMPLERATE;
        for (unsigned i=0; i<SAMPLERATE/10; i++) {
            double left = 0, right = 0;
            const double u = t + i * d;
            for (std::list<sound>::iterator it = p.begin();
                    it != p.end(); ++it)
            {
                stereo v = it->get(u);
                left += v.first;
                right += v.second;
            }
            a.push_back(left);
            a.push_back(right);
        }
    }
public:
    conductor(std::multiset<sound> & s) : s(s) {}
#   define UNTIL(c) while(!(c))
    void run()
    {
        unsigned k = 0;
        std::list<sound> p;
        UNTIL (p.empty() && s.empty()) {
            trim(.1 * (k + 1), p);
            std::cerr << "\r" << std::setw(3) << int(p.size()) << " @" << k;
            samples a;
            chunk(k * .1, p, a);
            o.write(a);
            ++k;
            const double z = k * .1;
            struct zbefore {
                const double t;
                bool operator()(sound & w) { return w.z <= t; }
            } b = { z };
            p.remove_if(b);
        }
        o.close();
        std::cerr << std::endl;
    }
};

struct lyricfile
{
    std::ofstream f;
    lyricfile(const char * path) : f(path) {}
    void put(double t, std::string & d)
    {
        std::string s;
        for (unsigned i=0; i<d.size(); i++)
            if (d[i] == '\n') s.append("\\n");
            else if (d[i] == '\r') s.append("\\r");
            else if (d[i] == '\\') s.append(">");
            else if (d[i] == '/') s.append("<");
            else if (strchr("\"`][", d[i])) s.push_back('\'');
            else s.push_back(d[i]);
        f << int(t * 100) << " \"" << s << "\"" << std::endl;
    }
};

int main(int argc, char **argv)
{
    if (argc <= 1) {
        std::cerr << "Give a MIDI file [and for lyrics a path to save at]" << std::endl;
        return 1;
    }
    if (isatty(1)) {
        std::cerr << "Redirect stdout.  Listen or to save respectively:\n"
            " | aplay -c 2 -f s16_be -r 44100 #or >tmp.raw #if slow\n"
            " | lame -r -s 44.1"
            " --signed --bitwidth 16 --big-endian"
            " - saved.mp3\n";
        return 1;
    }
    srand(time(nullptr));
    midi::file m(argv[1]);
    if (argc > 2) {
        std::vector<midi::event> y;
        m.syllables(y);
        lyricfile l(argv[2]);
        for (unsigned i=0; i<y.size(); i++) l.put(y[i].t, y[i].d);
    }
    orchestra h;
    std::multiset<sound> s;
    for (unsigned i=0; i<m.t.size(); i++) {
        for (unsigned j=0; j<m.t[i].c.size(); j++) {
            std::vector<midi::note> n;
            m.t[i].c[j].compile(n);
            for (unsigned k=0; k<n.size(); k++) {
                instrument * u = h.get_program(n[k].i);
                if (u) u->play(n[k], s);
            }
        }
    }
    conductor q(s);
    q.run();
}

