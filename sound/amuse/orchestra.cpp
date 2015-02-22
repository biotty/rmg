//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "orchestra.hpp"

#include "builders.hpp"
#include "musicm.hpp"
#include "math.hpp"

sound_entry mouthao(instruction ii)
{
    double a = 8 / ii.f;
    ii.d += a;

    pe_ptr ahe = P<punctual>();
    ahe->p(.5, 1);
    bu_ptr ah = P<harmonics>(P<still>(ii.f), ahe, 7, ii.p.get(0));
    bu_ptr bh = P<harmonics>(P<still>(ii.f), P<punctual>(.5, 0), 7, ii.p.get(0));
    bu_ptr s = P<cross>(ah, bh, P<movement>(P<punctual>(0, 1), ii.d));
    return sound_entry(P<attack>(a, ii.h, ii.d, s), a);
}

sound_entry fmfi(instruction ii)
{
    double a = 1 / ii.f;
    ii.d += a;

    pe_ptr ip = P<punctual>(ii.p.get(0), 0);
    mv_ptr ie = P<stroke>(ip, (ii.p.get(1)+.5) * ii.d, ii.d);
    bu_ptr ms = P<wave>(P<still>(ii.f * (ii.p.get(2)+.5)), P<sine>(0));
    bu_ptr s = P<fm>(ms, ie, P<still>(ii.f));
    return sound_entry(P<attack>(a, ii.h, ii.d, s), a);
}

sound_entry guitar(instruction ii)
{
    double a = 4 / ii.f;
    ii.d += a;

    bu_ptr s = P<karpluss_strong>(P<still>(ii.f),
            ii.p.get(0), ii.p.get(1));
    return sound_entry(P<attack>(a, ii.h, ii.d, s), a);
}

sound_entry drop(instruction ii)
{
    const double p = 1 / ii.f;
    const double d = p * 9 + .1;
    const double b = ii.f * .1;
    const double u = ii.f * 5 + 900;
    pe_ptr pe = P<punctual>(log(b), log(u));
    en_ptr en = P<added>(P<expounded>(pe), P<constant>(ii.f - b));
    mv_ptr mf = P<movement>(en, d);
    bu_ptr sw = P<wave>(mf, P<sine>(0));
    at_ptr aw = P<attack>(p, ii.h, d, sw);
    return sound_entry(aw);
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
    bu_ptr wc = P<wave>(P<still>(ii.f), we);

    return sound_entry(P<attack>(a, ii.h, ii.d, wc), a);
}

sound_entry fspread(instruction ii)
{
    double a = .05;
    ii.d += a;
    pe_ptr t = P<punctual>(c_at(ii.f) * 2, c_at(ii.f) * .5);
    bu_ptr s = P<chorus>(P<still>(ii.f, .3), t, P<sine>(0), 4);
    return sound_entry(P<attack>(a, ii.h, ii.d, s), a);
}

bu_ptr echo(bu_ptr b, mv_ptr m, double u, double x)
{
    fl_ptr l = P<strong>(.5, (1-x) * .5);
    fl_ptr lf = P<feedback>(l, P<movement>(
                P<scaled>(m->e, 1.0/f_of(100)), m->s),
            P<constant>((u+1)/20));
    return P<timed_filter>(b, lf, m->s);
}

bu_ptr comb(bu_ptr b, mv_ptr m, double u, double /*x*/)
{
    fl_ptr lf = P<feed>(P<as_is>(), P<still>((u+1)/20, m->s),
            P<inverted>(m->e));
    return P<timed_filter>(b, lf, m->s);
}

sound_entry::sound_entry (bu_ptr s, double a) : s(s), a(a) {}
sound_entry::sound_entry(bu_ptr s) : s(s), a() {}
sound_entry::sound_entry() : a() {}
sound_entry::operator bool() { return bool(s); }

instruction::instruction(double f, double d, double h, instruction::params p)
    : f(f), d(d), h(h), p(p) {}

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

bu_ptr orchestra::effect(unsigned i, bu_ptr b, mv_ptr m, double u, double x)
{
    if ( ! is_effect(i)) throw nullptr;
    return e[(i-50) % e.size()](b, m, u, x);
}

