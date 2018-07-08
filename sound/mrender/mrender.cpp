//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "midi.hpp"
#include "biquad.hpp"
#include "musicm.hpp"
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

#ifdef USE_JUST_INTONATION
#define F_OF_JUST f_of_just
#else // equal temperament
#define F_OF_JUST f_of
#endif

double const lim_amp = .8;
double const amp_per_sec = 1;
double const dist_amp = .995;
double const ear_secs = .00065;
double const pi = 3.1415926535;
double sign(double r) { return r >= 0 ? 1 : -1; }
double linear(double a, double b, double r) { return a * (1 - r) + b * r; }
double rnd(double a, double b) { return a + rand() * (b - a) / RAND_MAX; }

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
struct whitenoisefun : wavefun { double operator()(double) final { return rnd(-1, 1); } };
struct sinefun : wavefun { double operator()(double x) final { return sine(x); } };
struct squarefun : wavefun { double operator()(double x) final { return square(x); } };
struct sawtoothfun : wavefun { double operator()(double x) final { return sawtooth(x); } };

struct qshapefun : wavefun
{
    qshapefun(double a) : q(pow(10, -a)) {}
    double operator()(double x) final { return squareshape(sine(x), q); }
private:
    double q;
};

struct wshapefun : wavefun
{
    wshapefun(double a) : w(pow(10, -a)) {}
    double operator()(double x) final { return sine(sawtoothpull(x, w)); }
private:
    double w;
};

struct qwshapefun : wavefun
{
    qwshapefun(double a, double b)
        : q(pow(10, -a)), w(pow(10, -b))
    {}
    double operator()(double x) final {
        return squareshape(sine(sawtoothpull(x, w)), q);
    }
private:
    double q, w;
};

typedef std::vector<float> samples;

double enveloping_amplitude(samples const & s)
{
    double r = 0;
    for (float b : s) {
        const float a = fabsf(b);
        if (a > r) r = a;
    }
    return r;
}

class sampler
{
    samples c;
    double i, s, g;
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
public:
    sampler(double i = amp_per_sec/10) : i(i), s(), g() {}
    void write(samples w)
    {
        double a = enveloping_amplitude(w);
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
    void close()
    {
        if (c.empty()) return;
        flush(0);
        c.resize(0);
    }
};

typedef std::shared_ptr<wavefun> wavefunptr;

wavefunptr defaultfun() { return wavefunptr(new sinefun); }

struct vibrato
{
    vibrato() : a(), f(1), s(defaultfun()) {}
    vibrato(double p, double e, double f, wavefunptr s = defaultfun()) : f(f), s(s)
    {
        const double q = f_of(p);
        a = (f_of(p + e) - q) / q;
        // org: could use variant of c_of
    }
    double get(double x)
    {
        return 1 + a * (*s)(x * f);
    }
private:
    double a;
    double f;
    wavefunptr s;
};

struct tremolo
{
    tremolo() : k(), f(1), s(defaultfun()) {}
    tremolo(double l, double f, wavefunptr s = defaultfun())
        : k((1 - a_of(l)) / 2), f(f), s(s)
    {}
    double get(double x)
    {
        return 1 + k * ((*s)(x * f) - 1);
    }
private:
    double k;
    double f;
    wavefunptr s;
};

struct envelope
{
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
private:
    double a, d, s, r, k, u, q;
    double f(double x)
    {
        return (x < a) ? x / a
                : (x < a + d)
                    ? linear(1, s, (x - a) / d)
                    : s;
    }
};

struct sourcewave
{
    virtual double get(double d) = 0;
    sourcewave() : x() {}
protected:
    virtual void start() {}
    void advance(double d)
    {
        x += d;
        if (x >= 1) { x -= 1; start(); }
        else if (x < 0) x += 1;
    }
    double x;
};

struct funwave : sourcewave
{
    funwave(wavefunptr s = defaultfun()) : s(s) {}
    double get(double d)
    {
        double r = (*s)(x);
        advance(d);
        return r;
    }
private:
    wavefunptr s;
};

struct bandgrainwave : sourcewave
{
    bandgrainwave(double b) : b(b), e() { start(); x = rnd(0, 1); }
    double get(double d)
    {
        double r = sine(x);
        advance(d * e);
        return r;
    }
protected:
    void start() final { e = rnd(1, 1 + b); }
private:
    double b;
    double e;
};

struct bandnoisewave : sourcewave
{
    bandnoisewave(unsigned n, double b) : w(n, b) {}
    double get(double d)
    {
        double r = 0;
        for (unsigned i=0; i<w.size(); i++) r += w[i].get(d);
        return r / w.size();
    }
private:
    std::vector<bandgrainwave> w;
};

struct modulatorwave : private sourcewave
{
    modulatorwave(double t, double a, wavefunptr s = defaultfun()) : t(t), a(a), s(s) {}
    double get(double d)
    {
        const double r = a * (*s)(x);
        advance(d * t);
        return r;
    }
private:
    double t, a;
    wavefunptr s;
};

struct fmwave : sourcewave
{
    fmwave(double r, double f, wavefunptr s = defaultfun())
        : s(s), m(f, r)
    {}
    double get(double d)
    {
        const double r = (*s)(x);
        advance(d * (1 + m.get(d)));
        return r;
    }
private:
    wavefunptr s;
    modulatorwave m;
};

struct stringwave : sourcewave
{
    stringwave(double p, wavefunptr s, filter::biquad f)
        : f(f), s(s), a(SAMPLERATE / F_OF_JUST(p)), i()
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
private:
    filter::biquad f;
    wavefunptr s;
    samples a;
    unsigned i;
};

struct participantwave : private sourcewave
{
    participantwave(double t, wavefunptr s = defaultfun()) : t(t), s(s)
    { x = rnd(0, 1); }
    double get(double d)
    {
        const double r = (*s)(x);
        advance(d * t);
        return r;
    }
private:
    double t;
    wavefunptr s;
};

struct choruswave : sourcewave
{
    choruswave(std::vector<wavefunptr> fs, double t = .01)
    { for (wavefunptr f : fs) a.push_back(participantwave(1 + t, f)); }
    double get(double d)
    {
        double r = 0;
        for (unsigned i=0; i<a.size(); i++) r += a[i].get(d);
        return r / (a.size());
    }
private:
    std::vector<participantwave> a;
};

struct formant
{
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

struct harmonicwave : sourcewave
{
    harmonicwave(std::vector<formant> o, bool p = false)
        : p(p)
    {
        unsigned n = 0;
        for (formant & ft: o) {
            const double z = ft.z();
            if (n < z)
                n = z;
        }
        // obervation: we use overtone-n semantics instead of frequency, which would
        // be more readily adapted to phenomena like the human vocal tract, as resonance
        // it what mainly causes the formants.  the conversion is of-course pitch-
        // dependent, so instrument needs to consider when note play()ed
        a.resize(n);
        for (unsigned i=0; i<n; i++) {
            double ymax = 0;
            for (formant & ft: o) {
                const double y = ft.get(i);
                if (ymax < y)
                    ymax = y;
            }
            a[i] = ymax;
        }
    }
    double get(double d)
    {
        // observation: may seem inefficient to generate each repeated period,
        // the alternative would be to pre-generate a period and then loop over it,
        // however this may give aliasing since d is subject to vibrato unless
        // generated with a much higher sample-rate or we anti-alias.  we here choose
        // a solution putting importance on simplicity and accuracy.
        double r = 0;
        const unsigned i = p ? 2 : 1;
        for (unsigned q=0; q<a.size(); q+=i)
            r += a[q] * sine(x * (q + 1));
        advance(d);
        return r / a.size();
    }
private:
    bool p;
    std::vector<double> a;
};

typedef std::shared_ptr<sourcewave> waveptr;

typedef std::pair<double, double> stereo;

class sound
{
    bool fromright;
    unsigned n;
    double d, a, m;
    waveptr w;
    vibrato v;
    tremolo r;
    envelope e;
    filter::biquad f;
    filter::biquad g;
    filter::delay h;
public:
    double t;
    double z;
    sound(double t, double p, double l, double o, waveptr w,
            vibrato v, tremolo r, envelope e)
        : d(F_OF_JUST(p) / SAMPLERATE), a(a_of(l))
        , m(linear(dist_amp, 1, fabs(sin(o))))
        , w(w), v(v), r(r), e(e), t(t)
    {
        double c = cos(o);
        fromright = c > 0;
        double fardelay = fabs(c) * ear_secs;
        z = t + e.span() + fardelay;
        n = fardelay * SAMPLERATE;

        const double frombehind = .3 * (1 - sin(o));
        const double fromside = frombehind + .4 * fabs(cos(o));
        f.lowpass(linear(8e+3, 8e+2, frombehind));
        g.lowpass(linear(8e+3, 8e+2, fromside));
    }
    void alloc() { h.alloc(n); }
    stereo get(double s)
    {
        if (s < t || s > z) return stereo(0, 0);
        s -= t;
        const double y = a * e.get(s) * r.get(s) * w->get(d * v.get(s));
        const double near = f.shift(y);
        const double far = m * g.shift(h.shift(y));
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
        envelope e = envelope(0, .01, .5, .01, .04);
        funwave * y = new funwave(wavefunptr(new whitenoisefun));
        p.insert(sound(n.t, 33, n.l - 15, n.o, waveptr(y), v, r, e));
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
        std::vector<wavefunptr> a = {
            wavefunptr(new sawtoothfun),
            wavefunptr(new squarefun) };
        choruswave * y = new choruswave(a, 2);
        p.insert(sound(n.t, 33, n.l - 3, n.o, waveptr(w), v, r, ew));
        p.insert(sound(n.t, 21, n.l - 3, n.o, waveptr(y), v, r, ey));
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
        fmwave * y = new fmwave(1.5, 1, w());
        p.insert(sound(n.t, n.p, n.l - 3, n.o, waveptr(y), v, r, es));
    }
};

class vowel : public instrument
{
public:
    void play(midi::note n, std::multiset<sound> & p)
    {
        vibrato v(n.p, rnd(0.05, .1), rnd(3, 8));
        tremolo r(-2, rnd(3, 8));
        envelope e = envelope(0, 0, 1, n.d, rnd(.1, .2));
        std::vector<formant> f;
        f.push_back(formant(rnd(0, 4.5), rnd(.5, .9), rnd(0, 2), rnd(3, 6), rnd(3, 6)));
        f.push_back(formant(rnd(6, 9.5), rnd(.5, .7), rnd(0, 2), rnd(4, 7), rnd(4, 7)));
        f.push_back(formant(rnd(12, 15.5), rnd(.1, .5), rnd(0, 2), rnd(5, 8), rnd(5, 8)));
        harmonicwave * x = new harmonicwave(f, true);
        p.insert(sound(n.t, n.p, n.l, n.o, waveptr(x), v, r, e));
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
        p.insert(sound(n.t, rnd(70, 117), n.l - 12, n.o, waveptr(x), v, r, ex));
        p.insert(sound(n.t, rnd(70, 117), n.l - 18, n.o, waveptr(y), v, r, ey));
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
        envelope e = envelope(0.1, 0, 1, n.d, 0.6);
        filter::biquad f;
        f.lowpass(4 * f_of(n.p));
        stringwave * x = new stringwave(n.p, w(), f);
        p.insert(sound(n.t, 0, n.l, n.o, waveptr(x), v, r, e));
    }
};

// consider: parse orchestra instead of hard in program, and thus find
// apropriate for standard midi code-points.  arrange above stuff in
// indepedent files (building-blocks, instruments and parsing).
struct orchestra
{
    drum d;
    wind w;
    hihat h;

    string g;
    synth s;
    vowel v;

    instrument * get_program(midi::codepoint i)
    {
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
    if (isatty(1) && argc == 2) {
        std::cerr << "Redirect stdout i.e '> raw'.  Examples\n"
            "|lame -r -s 44.1 --signed --bitwidth 16 --big-endian - x.mp3\n"
            "aplay -c 2 -f s16_be -r 44100 raw\n";
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
    if (isatty(1)) {
        std::cerr << "\nNot generating audio because stdout is a tty\n";
        return 0;
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

