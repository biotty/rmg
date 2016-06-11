//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "orchestra.hpp"

#include "builders.hpp"
#include "musicm.hpp"
#include "math.hpp"

namespace {

struct attack : builder
{
    bu_ptr w;
    attack(double h, double y1, double t, bu_ptr && w)
        : w(U<sound>(std::move(w), t, make_stroke(P<punctual>(0, y1), h, t)))
    {}

    ug_ptr build() { return w->build(); }
};

}

sound_entry mouthao(instruction ii)
{
    double a = 8 / ii.f;
    ii.d += a;

    const double m = 3000;
    pe_ptr ahe = P<punctual>();
    ahe->p(.5, 1);
    bu_ptr ah = U<harmonics>(P<constant>(ii.f),
            m, P<stretched>(ahe, m), ii.p.get(0));
    bu_ptr bh = U<harmonics>(P<constant>(ii.f),
            m, P<stretched>(P<punctual>(.5, 0), m), ii.p.get(0));
    bu_ptr s = U<cross>(std::move(ah), std::move(bh), ii.d,
            P<stretched>(P<punctual>(0, 1), ii.d));
    return sound_entry(U<attack>(a, ii.h, ii.d, std::move(s)), a);
}

sound_entry fmfi(instruction ii)
{
    double a = 1 / ii.f;
    ii.d += a;

    pe_ptr ip = P<punctual>(ii.p.get(0), 0);
    en_ptr ie = make_stroke(ip, (ii.p.get(1)+.5) * ii.d, ii.d);
    bu_ptr ms = U<wave>(P<constant>(ii.f * (ii.p.get(2)+.5)), P<sine>(0));
    bu_ptr s = U<fm>(std::move(ms), ii.d, ie, P<constant>(ii.f));
    return sound_entry(U<attack>(a, ii.h, ii.d, std::move(s)), a);
}

sound_entry guitar(instruction ii)
{
    double a = 4 / ii.f;
    ii.d += a;

    bu_ptr s = U<karpluss_strong>(P<constant>(ii.f),
            ii.p.get(0), ii.p.get(1));
    return sound_entry(U<attack>(a, ii.h, ii.d, std::move(s)), a);
}

sound_entry drop(instruction ii)
{
    const double p = 1 / ii.f;
    const double d = p * 9 + .1;
    const double b = ii.f * .1;
    const double u = ii.f * 5 + 900;
    pe_ptr pe = P<punctual>(log(b), log(u));
    en_ptr en = P<added>(P<shaped>(pe, [](double x){ return exp(x); }),
            P<constant>(ii.f - b));
    bu_ptr sw = U<wave>(P<stretched>(en, d), P<sine>(0));
    return sound_entry(U<attack>(p, ii.h, d, std::move(sw)));
}

sound_entry shaperw(instruction ii)
{
    struct q_fun
    {
        double p;
        double operator()(double y) { return spow(y, p); }
    } qf = { exp(linear(-2, 2, ii.p.get(0))) };

    struct w_fun
    {
        double p;
        double operator()(double x)
        {
            if (x < 0) x += 1;
            return (spow(2 * (x - .5), p) + 1) * .5;
        }
    } wf = { exp(linear(-2, 2, ii.p.get(1))) };

    double a = 1 / ii.f;
    ii.d += a;

    en_ptr se = P<sine>(0);
    en_ptr qe = P<shaped>(se, qf);
    en_ptr we = P<warped>(qe, wf);
    bu_ptr wc = U<wave>(P<constant>(ii.f), we);
    return sound_entry(U<attack>(a, ii.h, ii.d, std::move(wc)), a);
}

sound_entry fspread(instruction ii)
{
    double a = .05;
    ii.d += a;
    pe_ptr t = P<punctual>(c_at(ii.f) * 2, c_at(ii.f) * .5);
    bu_ptr s = U<chorus>(P<constant>(ii.f), t, P<sine>(0), 4);
    return sound_entry(U<attack>(a, ii.h, ii.d, std::move(s)), a);
}

bu_ptr echo(bu_ptr && b, mv_ptr m, double u, double /*x*/)
{
    fl_ptr lf = P<feedback>(
            P<stretched>(P<shaped>(m->e,
                    [](double x){ return x / f_of(100); }), m->s),
            P<constant>((u+1)/20));
    return U<timed_filter>(std::move(b), lf, m->s);
}

bu_ptr comb(bu_ptr && b, mv_ptr m, double u, double /*x*/)
{
    fl_ptr lf = P<feed>(
            P<constant>((u+1)/20),
            P<stretched>(P<shaped>(m->e, inverts()),
                m->s));
    return U<timed_filter>(std::move(b), lf, m->s);
}

sound_entry::sound_entry(bu_ptr && s, double a) : s(std::move(s)), a(a) {}
sound_entry::sound_entry(bu_ptr && s) : s(std::move(s)), a() {}
sound_entry::sound_entry() : a() {}
sound_entry::operator bool() { return bool(s); }

instruction::instruction(double f, double d, double h, instruction::params p)
    : f(f), d(d), h(h), p(p)
{}

orchestra::orchestra() : a({
            mouthao,
            fmfi,
            guitar,
            drop,
            shaperw,
            fspread
        }), e({
            echo,
            comb
        })
{}

bool orchestra::is_effect(unsigned i)
{
    return i >= 50;
}

sound_entry orchestra::play(unsigned i, instruction ii)
{
    if (is_effect(i)) throw nullptr;
    return a[i % a.size()](ii);
}

bu_ptr orchestra::effect(unsigned i, bu_ptr && b, mv_ptr m, double u, double x)
{
    if ( ! is_effect(i)) throw nullptr;
    return e[(i-50) % e.size()](std::move(b), m, u, x);
}

