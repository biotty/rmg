//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "fugelib.hpp"
#include "generators.hpp"
#include "builders.hpp"
#include "musicm.hpp"
#include <map>

namespace fuge {

double tempo = 0.11;  // only written initially

long
parse_int(PyObject * n)
{
    PyObject * i = PyNumber_Long(n);
    const long x = PyLong_AsLong(i);
    Py_DECREF(i);
    return x;
}

std::string
parse_string(PyObject * s)
{
    if (PyObject * o = PyUnicode_AsLatin1String(s)) {
        std::string r = PyBytes_AsString(o);
        Py_DECREF(o);
        return r;
    } else {
        return "";
    }
}

double
parse_float(PyObject * n)
{
    if ( ! PyNumber_Check(n)) {
        PyObject * r = PyObject_Repr(n);
        const char * s = PyUnicode_AsUTF8(r);
        Py_DECREF(r);
        throw std::runtime_error(s);
    }
    PyObject * f = PyNumber_Float(n);
    const double x = PyFloat_AsDouble(f);
    Py_DECREF(f);
    return x;
}

struct param
{
    std::vector<double> values;
    double get() const { return values.at(0); }
};

param
parse_param(bool scalar_only, PyObject * o)
{
    param s;
    if (scalar_only || ! PyList_Check(o)) {
        s.values.push_back(parse_float(o));
    } else {
        const int k = PyList_Size(o);
        for (int j=0; j<k; j++)
            s.values.push_back(parse_float(PyList_GetItem(o, j)));
    }
    return s;
}

typedef std::vector<param> params;

params
parse_params(const char * e, PyObject * seq, int i = 0)
{
    params r;
    if ( ! PyList_Check(seq)) throw std::runtime_error("params isn't List");
    const int n = PyList_Size(seq);
    if (strlen(e) != unsigned(n)) throw std::runtime_error(e);
    for (; i<n; i++)
        r.push_back(parse_param(e[i] == '+', PyList_GetItem(seq, i)));
    return r;
}

en_ptr
mk_envelope(param const & p)
{
    std::vector<double> const & v = p.values;
    unsigned n = v.size();
    if ( ! n) throw std::runtime_error("list-param empty");
    if (n == 1) return P<constant>(v[0]);

    pe_ptr e = P<punctual>(v[0], v[n - 1]);
    for (unsigned i=1; i+1<n; i++) e->p(i /(double) n, v[i]);
    return e;
}

static bu_ptr parse_beep(en_ptr e, double duration, PyObject * list)
{
    const params p = parse_params("++@", list);
    en_ptr f = P<stretched>(mk_envelope(p[2]), duration);
    return U<trapesoid>(p[1].get() / p[2].get(), p[0].get(), duration,
            U<wave>(f, e));
}

struct instrument
{
    virtual bu_ptr operator()(double duration, PyObject * list) = 0;
    virtual ~instrument() {}
};

struct freqwave : instrument
{
    bu_ptr operator()(double duration, PyObject * list)
    {
        return parse_beep(P<sine>(0), duration, list);
    }
};

struct sawtooth : instrument
{
    bu_ptr operator()(double duration, PyObject * list)
    {
        return parse_beep(P<punctual>(1, -1), duration, list);
    }
};

struct square : instrument
{
    bu_ptr operator()(double duration, PyObject * list)
    {
        tabular * t = new tabular();
        t->values.push_back(1);
        t->values.push_back(-1);
        return parse_beep(en_ptr(t), duration, list);
    }
};

struct stair : instrument
{
    bu_ptr operator()(double duration, PyObject * list)
    {
        tabular * t = new tabular();
        t->values.push_back(1);
        t->values.push_back(0);
        t->values.push_back(-1);
        t->values.push_back(0);
        return parse_beep(en_ptr(t), duration, list);
    }
};

struct ks_string : instrument
{
    bu_ptr operator()(double duration, PyObject * list)
    {
        const params p = parse_params("++@++", list);
        en_ptr e = mk_envelope(p[2]);
        return U<attack>(p[1].get() / p[2].get(), p[0].get(), duration,
                U<karpluss_strong>(P<stretched>(e, duration),
                    p[3].get(), p[4].get()));
    }
};

struct diphthong : instrument
{
    bu_ptr operator()(double duration, PyObject * list)
    {
        const params p = parse_params("++@@+@+", list);
        en_ptr f = P<stretched>(mk_envelope(p[2]), duration);
        bu_ptr a = U<harmonics>(f, mk_envelope(p[3]), p[4].get(), 4000);
        bu_ptr b = U<harmonics>(f, mk_envelope(p[5]), p[6].get(), 4000);
        return U<attack>(p[1].get() / p[2].get(), p[0].get(), duration,
                U<cross>(std::move(a), std::move(b),
                    P<stretched>(P<punctual>(0, 1), duration)));
    }
};

struct fqm : instrument
{
    bu_ptr operator()(double duration, PyObject * list)
    {
        if (duration == 0) throw std::runtime_error("fqm misses duration");
        PyObject * head = PyList_GetItem(list, 0);
        bu_ptr m = parse_note(head, duration);
        const params p = parse_params("?++@@", list, 1);
        en_ptr index = mk_envelope(p[2]);
        en_ptr carrier = mk_envelope(p[3]);
        en_ptr i = P<stretched>(index, duration);
        en_ptr c = P<stretched>(carrier, duration);
        return U<attack>(p[1].get() / p[3].get(), p[0].get(), duration,
                U<fm>(std::move(m), i, c));
    }
};

struct amm : instrument
{
    bu_ptr operator()(double duration, PyObject * list)
    {
        if (duration != 0) throw std::runtime_error("amm with explicit duration");
        const int n = PyList_Size(list);
        if (n != 2) throw std::runtime_error("amm requires 2 notes");
        return U<am>(
                parse_note(PyList_GetItem(list, 0)),
                parse_note(PyList_GetItem(list, 1)));
    }
};

std::map<std::string, std::unique_ptr<instrument>> orchestra;

void
init_orchestra()
{
    orchestra.emplace("sine", std::unique_ptr<instrument>(new freqwave));
    orchestra.emplace("sawtooth", std::unique_ptr<instrument>(new sawtooth));
    orchestra.emplace("square", std::unique_ptr<instrument>(new square));
    orchestra.emplace("stair", std::unique_ptr<instrument>(new stair));
    orchestra.emplace("amp-mod", std::unique_ptr<instrument>(new amm));
    orchestra.emplace("freq-mod", std::unique_ptr<instrument>(new fqm));
    orchestra.emplace("diphthong", std::unique_ptr<instrument>(new diphthong));
    orchestra.emplace("ks-string", std::unique_ptr<instrument>(new ks_string));
}

bu_ptr
parse_note(PyObject * seq, double duration)
{
    if ( ! PyTuple_Check(seq)) throw std::runtime_error("note isn't Tuple");
    const int n = PyTuple_Size(seq);
    int i = 0;
    if (n == 0) throw std::runtime_error("note Tuple empty");
    PyObject * head = PyTuple_GetItem(seq, 0);
    if (PyNumber_Check(head)) {
        if (duration != 0) throw std::runtime_error("duration over-ridden");
        duration = tempo * parse_float(head);
        ++i;
    }
    if (i + 2 != n) throw std::runtime_error("note Tuple size invalid");
    std::string label = parse_string(PyTuple_GetItem(seq, i++));
    return (*orchestra.at(label))(duration, PyTuple_GetItem(seq, i++));
}

struct filter
{
    virtual bu_ptr operator()(double duration, bu_ptr && input, PyObject * list) = 0;
    virtual ~filter() {}
};

struct echo : filter
{
    bu_ptr operator()(double duration, bu_ptr && input, PyObject * list)
    {
        const params p = parse_params("@@", list);
        en_ptr amount = P<stretched>(mk_envelope(p[0]), duration);
        en_ptr delay = P<stretched>(mk_envelope(p[1]), duration);
        fl_ptr lf = P<feedback>(P<as_is>(), amount, delay);
        return U<timed_filter>(std::move(input), lf, duration);
    }
};

struct comb : filter
{
    bu_ptr operator()(double duration, bu_ptr && input, PyObject * list)
    {
        const params p = parse_params("@@", list);
        en_ptr amount = P<stretched>(mk_envelope(p[0]), duration);
        en_ptr delay = P<stretched>(mk_envelope(p[1]), duration);
        fl_ptr lf = P<feed>(P<as_is>(), amount, delay);
        return U<timed_filter>(std::move(input), lf, duration);
    }
};

struct biqd : filter
{
    bu_ptr operator()(double duration, bu_ptr && input, PyObject * list)
    {
        const params p = parse_params("@@@@@", list);
        biquad::control c;
        c.b0 = P<stretched>(mk_envelope(p[0]), duration);
        c.b1 = P<stretched>(mk_envelope(p[1]), duration);
        c.b2 = P<stretched>(mk_envelope(p[2]), duration);
        c.a1 = P<stretched>(mk_envelope(p[3]), duration);
        c.a2 = P<stretched>(mk_envelope(p[4]), duration);
        fl_ptr f = P<biquad>(c);
        return U<timed_filter>(std::move(input), f, duration);
    }
};

struct fmix : filter
{
    bu_ptr operator()(double duration, bu_ptr && input, PyObject * list)
    {
        const int n = PyList_Size(list);
        if (n == 0) throw std::runtime_error("empty fmix-list");
        trunk::ptr t = P<trunk>(std::move(input));
        for (int i=0; i!=n; ++i) {
            PyObject * f = PyList_GetItem(list, i);
            t->branch(parse_filter(U<trunk::leaf>(t), f, true));
        }
        return U<timed_filter>(t->conclude(), P<as_is>(), duration);
    }
};

std::map<std::string, std::unique_ptr<fuge::filter>> effects;

void
init_effects()
{
    effects.emplace("comb", std::unique_ptr<filter>(new comb));
    effects.emplace("echo", std::unique_ptr<filter>(new echo));
    effects.emplace("biquad", std::unique_ptr<filter>(new biqd));
    effects.emplace("mix", std::unique_ptr<filter>(new fmix));
    // todo: serial - list of no_duration filters applied sequentially
}

bu_ptr
parse_filter(bu_ptr && input, PyObject * seq, bool no_duration)
{
    const int n = PyTuple_Size(seq);
    int i = 0;
    const double duration = no_duration ? 1e9
        : tempo * parse_float(PyTuple_GetItem(seq, i++));
    if (n != i + 2) throw std::runtime_error("effect Tuple has invalid size");
    std::string label = parse_string(PyTuple_GetItem(seq, i++));
    return (*effects.at(label))(duration, std::move(input), PyTuple_GetItem(seq, i++));
}

}
