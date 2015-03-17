//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef MIDI_HPP
#define MIDI_HPP

#include <vector>
#include <string>
#include <fstream>

namespace midi {

struct codepoint
{
    bool is_percussion;
    int n;
    codepoint();
};

struct note // api users desire is to get these
{
    double t, p, d, l, o; // time pitch duration loudness orientation
    codepoint i;
    note();
};

struct event
{
    double t;
    unsigned c, h, o; // MIDI
    std::string d;
    event(double t, unsigned s);
    unsigned p();
    void parse(const unsigned char * s, unsigned & i);
};

struct channel
{
    unsigned i;
    std::vector<event> e;
    channel(unsigned i);
    void compile(std::vector<note> & s);
};

struct tempochange
{
    double s, t, m;
    tempochange(double t, double m);
    double mapped(double u);
};

typedef std::vector<tempochange> tempomap;

struct tempomapper
{
    tempomap & m;
    unsigned i;
    tempomapper(tempomap & m);
    void apply(double & t);
};

struct track
{
    std::vector<channel> c;
    std::vector<event> v;
    double k;
    bool y;
    track(std::ifstream & f, double k);
    tempomap get_map();
    void apply_map(tempomap & m);
private:
    void parse(unsigned char * a, unsigned n);
};

class file
{
    std::ifstream f;
public:
    std::vector<track> t;
    file(const char * path);
    void syllables(std::vector<event> & y);
};

}

#endif

