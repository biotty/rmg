//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef ORCHESTRA_HPP
#define ORCHESTRA_HPP

#include <memory>
#include <vector>
#include <functional>

#include <cstdint>

struct envelope;
typedef std::shared_ptr<envelope> en_ptr;
struct movement {
    en_ptr e; double s;
    movement(en_ptr e, double s) : e(e), s(s) {};
};
typedef std::shared_ptr<movement> mv_ptr;

struct builder;
typedef std::unique_ptr<builder> bu_ptr;

#define MAX_T_AHEAD .4
struct sound_entry
{
    bu_ptr s;
    double a;

    sound_entry(bu_ptr && s, double a);
    sound_entry(bu_ptr && s);
    sound_entry();
    operator bool();
};

struct instruction
{
    double f;
    double d;
    double h;

    instruction(double f, double d, double h);
};

class orchestra
{
    std::vector<std::function<sound_entry(instruction)>> a;
    std::vector<std::function<bu_ptr(bu_ptr &&, mv_ptr, double u, double t)>> e;

public:
    bool is_effect(unsigned i);
    sound_entry play(unsigned i, instruction ii);
    bu_ptr effect(unsigned i, bu_ptr && b, mv_ptr m, double u, double t);
    orchestra();
};

#endif

