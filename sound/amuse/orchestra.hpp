//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef ORCHESTRA_HPP
#define ORCHESTRA_HPP

#include <memory>
#include <vector>
#include <functional>

#include <cstdint>

struct movement;
struct builder;

typedef std::shared_ptr<movement> mv_ptr;
typedef std::shared_ptr<builder> bs_ptr;
typedef std::unique_ptr<builder> bu_ptr;

#define MAX_T_AHEAD .4
struct sound_entry
{
    bs_ptr s;
    double a;

    sound_entry(bs_ptr s, double a);
    sound_entry(bs_ptr s);
    sound_entry();
    operator bool();
};

#define N_PARAMS 8
struct instruction
{
    double f;
    double d;
    double h;
    struct params {
        uint8_t a[N_PARAMS];

        double get(unsigned i) { return a[i] /(double) 99; }
        params() { for (unsigned i=0; i!=N_PARAMS; i++) a[i] = 0; }
    };
    params p;

    instruction(double f, double d, double h, params p);
};

class orchestra
{
    std::vector<std::function<sound_entry(instruction)>> a;
    std::vector<std::function<bs_ptr(bs_ptr, mv_ptr, double u, double t)>> e;

public:
    bool is_effect(unsigned i);
    sound_entry play(unsigned i, instruction ii);
    bs_ptr effect(unsigned i, bs_ptr b, mv_ptr m, double u, double t);
    orchestra();
};

#endif

