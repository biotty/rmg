
#include "midi.hpp"

#include <cassert>
#include <cstring>
#include <iostream>

#define TRACKBYTES (1024*1024)

namespace {

double const micro = 0.000001;

unsigned bdword(const unsigned char * s)
{
    return s[0] << 24 | s[1] << 16 | s[2] << 8 | s[3];
}

unsigned decode(const unsigned char * s, unsigned & i)
{
    unsigned v = 0;
    for (;;) {
        unsigned b = s[i++];
        unsigned q = b & 0x80;
        v |= b ^ q;
        if (!q) break;
        v <<= 7;
    }
    return v;
}

double velocity_l(double v) { return 6 * (std::min<int>(v, 64) / 64.0 - 1); }
double volume_l(double v) { return 12 * (std::min<int>(v, 64) / 64.0 - 1); }
double pan_o(double v) { return 3.141592 * (1 - v / 127.0); }
unsigned wat(const unsigned char * s, unsigned i) { return s[i] * 256 + s[i + 1]; }

}

namespace midi {

instrument::instrument() : p(), n() {}
note::note() : t(), p(), d(), l(), o(), i() {}

event::event(double t, unsigned s) : t(t), c(s >> 4), h(s & 15), o() {}
unsigned event::p() { assert(d.size() == 1); return d[0]; }
void event::parse(const unsigned char * s, unsigned & i)
{
    assert(d.empty());
    if (c == 12 || c == 13) {
        o = s[i++];
    } else if (c == 15) {
        if (h == 15) {
            o = s[i++];
            unsigned n = s[i++];
            d.append((char *)(s + i), n);
            i += n;
        } else {
            std::cerr << "f" << std::hex << h << std::dec << std::endl;
        }
    } else {
        o = s[i++];
        d.push_back(s[i++]);
    }
}

channel::channel(unsigned i) : i(i) {}
void channel::compile(std::vector<note> & s)
{
    unsigned r = 0;
    double o = pan_o(63);
    double l = 0;
    for (unsigned k=0; k<e.size(); k++) {
        event & ek = e[k];
        switch (ek.c) {
            case 12:
                r = ek.o;
                if (i == 9) std::cerr << "c9 " << r << std::endl;
                break;
            case 11:
                if (ek.o == 10) o = pan_o(ek.p());
                else if (ek.o == 7) l = volume_l(ek.p());
                break;
            case 9:
                if (ek.p() == 0) break; //zero-velocity is note-off
                note n;
                n.t = ek.t;
                n.l = l + velocity_l(ek.p());
                n.o = o + 6.283 * (r / 256.0 + ek.t / 60);
                if (i == 9) {
                    n.i.n = ek.o;
                    n.i.p = true;
                    n.p = 33;
                } else {
                    n.i.n = r;
                    n.p = ek.o;
                    double d = 16; //some max-duration
                    for (unsigned z=k+1; z<e.size(); z++) {
                        if (e[z].c == 8 || e[z].c == 9) {
                            if (e[z].t - ek.t > d)
                                break;
                            if (e[z].o == ek.o) {
                                d = e[z].t - ek.t;
                                break;
                            }
                        }
                    }
                    n.d = d;
                }
                s.push_back(n);
        }
    }
}

tempochange::tempochange(double t, double m) : s(), t(t), m(m) {}
double tempochange::mapped(double u) { return (u - t) * m + s; }

tempomapper::tempomapper(tempomap & m) : m(m), i()
{ 
    if ( ! m.empty()) m[0].s = m[0].t * .5;
}

void tempomapper::apply(double & t)
{
    unsigned c = m.size();
    if (c && (i || t >= m[0].t)) {
        if (i + 1 < c && t >= m[i + 1].t) {
            ++i;
            m[i].s = m[i - 1].mapped(m[i].t);
        }
        t = m[i].mapped(t);
    } else t *= .5;
}

track::track(std::ifstream & f, double k) : k(k), y()
{
    for (unsigned i=0; i<16; i++) c.push_back(channel(i));
    unsigned char h[8];
    f.read((char *)h, 8);
    if ( ! f ) throw 'f';
    const unsigned n = bdword(h + 4);
    if (memcmp(h, "MTrk", 4)) {
        f.seekg(n, f.cur);
        std::cerr << "alien track" << (int)h[0] << "."<<(int)h[1] << std::endl;
        return;
    }
    assert(n < TRACKBYTES);
    unsigned char * d = new unsigned char[n];
    f.read((char *)d, n);
    if ( ! f ) throw 0;
    y = true;
    parse(d, n);
    delete[] d;
}

tempomap track::get_map()
{
    tempomap r;
    for (unsigned i=0; i<v.size(); i++) {
        event & e = v[i];
        if (e.h == 15 && e.o == 81) {
            assert(e.d.size() == 3);
            const unsigned p = ((unsigned char)e.d[0]) * 65536
                + ((unsigned char)e.d[1]) * 256 + ((unsigned char)e.d[2]);
            r.push_back(tempochange(e.t, p * micro));
            std::cerr << "tempo: " << p << " micros per quarternote"
                << std::endl;
        }
    }
    return r;
}

void track::apply_map(tempomap & m)
{
    for (unsigned i=0; i<c.size(); i++) {
        tempomapper a(m);
        for (unsigned j=0; j<c[i].e.size(); j++)
            a.apply(c[i].e[j].t);
    }
    tempomapper b(m);
    for (unsigned k=0; k<v.size(); k++) b.apply(v[k].t);
}

void track::parse(unsigned char * a, unsigned n)
{
    double t = 0;
    unsigned i = 0, s = 0;
    while (i < n) {
        t += k * decode(a, i);
        unsigned b = a[i++];
        if (b & 0x80) s = b; else --i;
        event e(t, s);
        e.parse(a, i);
        if (e.c == 15) v.push_back(e);
        else c[e.h].e.push_back(e);
    }
}

file::file(const char * path) : f(path, std::ios::binary)
{
    unsigned char s[14];
    f.read((char *)s, 14);
    if ( ! f || memcmp(s, "MThd", 4)) {
        std::cerr << "not MIDI file" << std::endl;
        return;
    }
    unsigned x = bdword(s + 4);
    unsigned m = wat(s, 8);
    unsigned n = wat(s, 10);
    unsigned q = wat(s, 12);
    if (q & 0x8000) {
        std::cerr << "time-frame format not implemented" << std::endl;
        q = 1000;
    }
    if (x > 6) f.seekg(x - 6, f.cur);
    std::cerr << "mid" << m << " #" << n
        << " @" << q << std::endl;
    tempomap h;
    for (unsigned i=0; i<n; i++) {
        std::cerr << "\r[" << i << "]: ";
        track c(track(f, 1 /(double) q));
        if (c.y) {
            if (i == 0) h = c.get_map();
            t.push_back(c);
            t.back().apply_map(h);
        }
    }
}

void file::syllables(std::vector<event> & y)
{
    assert(y.empty());
    for (unsigned i=0; i<t.size(); i++) {
        for (unsigned j=0; j<t[i].v.size(); j++) {
            event & e = t[i].v[j];
            if (e.o == 5) y.push_back(e);
        }
        if ( ! y.empty()) break;
    }
}

}

