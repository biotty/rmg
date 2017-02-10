//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "midi.hpp"

#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>


namespace {


constexpr int featured_rotation_rpm = 0;

constexpr double o_of_featured_rotation(int r, double t)
{
    return 6.283 * (r / 256.0 + t / 60)
        * featured_rotation_rpm;
}

const double micro = 0.000001;

unsigned be16at(const unsigned char * s) { return s[0] << 8 | s[1]; }

unsigned be32at(const unsigned char * s) { return be16at(s) << 16 | be16at(s + 2); }

unsigned decode(const unsigned char * s, unsigned & i)
{
    // number represented with 7bit base in bytes with msb as more-indicator
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


}


namespace midi {


codepoint::codepoint() : is_percussion(), n() {}
note::note() : t(), p(), d(), l(), o(), i() {}

event::event(double t, unsigned status)
    : t(t), command(status >> 4), ch(status & 15), op()
{}

void event::decode(const unsigned char * s, unsigned & i)
{
    assert(d.empty());
    if (command == 12 || command == 13) {
        // program-change, after-touch
        op = s[i++];  // ..have only one operand
    } else if (command == 15) {
        if (ch == 15) {  // meta event
            op = s[i++];
            unsigned n = ::decode(s, i);
            d.append((char *)(s + i), n);
            i += n;
        } else if (ch == 0) {  // system-exclusive event
            op = s[i++];  // "id" per manufactorer
            unsigned char b;
            while (0 == ((b = s[i++]) & 0x80))
                d.push_back(b);
            if (b != 0xf7) {
                std::cerr << "sys-ex of length " << d.size()
                    << " was terminated by "
                    << std::hex << b << std::dec << std::endl;
            }
        } else if (ch == 2) {  // song-position
            op = s[i++];
            d.push_back(s[i++]);
        } else if (ch == 3) {  // song-select
            op = s[i++];
        } else if (ch == 6) {  // tune-request
        } else if (ch <= 7) {
            std::cerr << "undefined sys-ex f"
                << std::hex << ch << std::dec << std::endl;
        } else {
            std::cerr << "real-time-only sys-ex f"
                << std::hex << ch << std::dec << std::endl;
        }
    } else {  // all other commands have two operands
        op = s[i++];
        d.push_back(s[i++]);
    }
}

unsigned event::op2() { assert(d.size() == 1); return d[0]; }

channel::channel(unsigned i) : i(i) {}

void channel::compile(std::vector<note> & s)
{
    unsigned r = 0;
    double o = pan_o(63);
    double l = 0;
    for (unsigned k=0; k<e.size(); k++) {
        event & ek = e[k];
        switch (ek.command) {
            case 12:  // program-change
                r = ek.op;
                if (i == 9)
                    std::cerr << "program-change " << r
                        << " on percussion-channel" << std::endl;
                break;
            case 11:  // control-change
                if (ek.op == 10) o = pan_o(ek.op2());
                else if (ek.op == 7) l = volume_l(ek.op2());
                break;
            case 9:  // note-on
                if (ek.op2() == 0) break;  // zero-velocity equals a note-off
                note n;
                n.t = ek.t;
                n.l = l + velocity_l(ek.op2());
                n.o = o + o_of_featured_rotation(r, ek.t);
                if (i == 9) {
                    n.i.n = ek.op;
                    n.i.is_percussion = true;
                    n.p = 33;  // percussion-instrument *should* not use pitch
                } else {
                    n.i.n = r;
                    n.p = ek.op;
                    double d = 0;
                    for (unsigned z=k+1; z<e.size(); z++) {
                        if (e[z].command == 8 || e[z].command == 9) { //on-off
                            if (e[z].t - ek.t > 16) {
                                d = 16;  // *some* max-duration
                                break;
                            }
                            if (e[z].op == ek.op) {  // equal note-number
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
    const int trackpos = f.tellg();

    for (unsigned i=0; i<16; i++)
        c.push_back(channel(i));

    unsigned char header[8];
    f.read((char *)header, sizeof header);
    unsigned n = 0;

    const char * err = nullptr;
    if ( ! f) err = "eof header";
    else if (memcmp(header, "MTrk", 4)) err = "alien";
    else if ((n = be32at(header + 4)) > 1000000) err = "jumbo";
    else {
        auto body = std::unique_ptr<unsigned char[]>
                               (new unsigned char[n]);
        f.read((char *)body.get(), n);
        if ( ! f) err = "eof";
        else parse(body.get(), n);
    }

    if (err) {
        std::cerr << err << " track at position" << trackpos << std::endl;
        if (f && n) f.seekg(n, f.cur);
    } else {
        y = true;
    }
}

tempomap track::get_map()
{
    tempomap r;
    for (unsigned i=0; i<v.size(); i++) {
        event & e = v[i];
        if (e.ch == 15 && e.op == 81) {  // 0x51 tempo
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
    unsigned i = 0, status = 0;
    while (i < n) {
        t += k * decode(a, i);
        unsigned b = a[i];
        if (b & 0x80) {
            status = b;
            ++i;
        }
        event e(t, status);
        e.decode(a, i);
        if (e.command == 15) v.push_back(e);
        else c[e.ch].e.push_back(e);
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
    unsigned x = be32at(s + 4);
    unsigned m = be16at(s + 8);
    unsigned n = be16at(s + 10);
    unsigned q = be16at(s + 12);
    if (q & 0x8000) {
        std::cerr << "time-frame format not implemented" << std::endl;
        q = 1000;
    }
    if (x > 6) f.seekg(x - 6, f.cur);
    std::cerr << "mid" << m << " #" << n
        << " *" << q << std::endl;
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
            if (e.op == 5 || (e.op == 1 && e.t > 0))
                y.push_back(e);
        }
        if ( ! y.empty()) break;
    }
}


}
