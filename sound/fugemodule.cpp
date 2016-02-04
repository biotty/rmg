//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "Python.h"
#include "unit.hpp"
#include "generators.hpp"
#include "builders.hpp"
#include "musicm.hpp"
#include <stdexcept>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <cstdint>
#include <cassert>
#include <cmath>

namespace {

double tempo;

struct UgenObject {
    PyObject_HEAD
    generator * g;
};

PyObject *
Ugen_new(PyTypeObject * type, PyObject * args, PyObject * kw)
{
    PyObject * self = type->tp_alloc(type, 0);
    UgenObject & u = *reinterpret_cast<UgenObject *>(self);
    u.g = nullptr;
    return self;
}

void
Ugen_dealloc(PyObject * self)
{
    UgenObject & u = *reinterpret_cast<UgenObject *>(self);
    delete u.g;
    self->ob_type->tp_free(self);
}

PyObject *
Ugen_call(PyObject * self, PyObject * args, PyObject * kw)
{
    UgenObject & u = *reinterpret_cast<UgenObject *>(self);
    int16_t e[unit::size];
    int z = sizeof e;
    if (u.g && u.g->more()) {
        unit a;
        u.g->generate(a);
        std::transform(std::begin(a.y), std::end(a.y),
                std::begin(e),
                [](double q) -> int16_t { return 32766 * q; });
    } else {
        z = 0;
    }
    return PyBytes_FromStringAndSize((char *)e, z);
}

PyTypeObject UgenType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "fuge.Ugen", sizeof (UgenObject), 0, Ugen_dealloc,
    0, 0, 0, 0, 0, 0, 0, 0, 0, Ugen_call,
    0, 0, 0, 0, Py_TPFLAGS_DEFAULT, "encoded-unit generator",
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, Ugen_new,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

long
parse_int(PyObject * n)
{
    PyObject * i = PyNumber_Long(n);
    const long x = PyLong_AsLong(i);
    Py_DECREF(i);
    return x;
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
    std::size_t size() const { return values.size(); }
};

typedef std::vector<param> params;

params
parse_params(const char * e, PyObject * seq, int i = 0)
{
    params r;
    if ( ! PyList_Check(seq)) throw std::runtime_error("params isn't List");
    const int n = PyList_Size(seq);
    if (strlen(e) != unsigned(n)) throw std::runtime_error(e);
    for (; i<n; i++) {
        PyObject * o = PyList_GetItem(seq, i);
        param s;
        if ( e[i] == '+' || ! PyList_Check(o)) {
            s.values.push_back(parse_float(o));
        } else {
            const int k = PyList_Size(o);
            for (int j=0; j<k; j++)
                s.values.push_back(parse_float(PyList_GetItem(o, j)));
        }
        r.push_back(s);
    }
    return r;
}

struct instrument
{
    virtual bu_ptr operator()(double duration, PyObject * list) = 0;
    virtual ~instrument() {}
};

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

bu_ptr parse_note(PyObject * seq, double span = 0);

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

namespace fuge { // because ::filter exists

struct filter
{
    virtual bu_ptr operator()(double duration, bu_ptr && input, PyObject * list) = 0;
    virtual ~filter() {}
};

}

struct echo : fuge::filter
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

struct comb : fuge::filter
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

struct biqd : fuge::filter
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

bu_ptr parse_filter(bu_ptr && input, PyObject * seq, bool no_duration = false);

struct fmix : fuge::filter
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
    effects.emplace("comb", std::unique_ptr<fuge::filter>(new comb));
    effects.emplace("echo", std::unique_ptr<fuge::filter>(new echo));
    effects.emplace("biquad", std::unique_ptr<fuge::filter>(new biqd));
    effects.emplace("mix", std::unique_ptr<fuge::filter>(new fmix));
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

bu_ptr
parse_score(PyObject * seq);

struct score_entry {
    double t;
    bu_ptr b;
    score_entry() : t() {}
    score_entry(double t, PyObject * data) : t(t)
    {
        if (PyTuple_Check(data)) b = parse_note(data);
        else if (PyList_Check(data)) b = parse_score(data);
        else throw std::runtime_error(
                "score entry data isn't Tuple or List");
    }
};

score_entry
parse_entry(PyObject * seq)
{
    if ( ! PyTuple_Check(seq)) throw std::runtime_error("score entry isn't Tuple");
    const int n = PyTuple_Size(seq);
    if (n != 2) throw std::runtime_error("score entry Tuple doesn't have size 2");
    PyObject * head = PyTuple_GetItem(seq, 0);
    const double t = parse_float(head);
    return score_entry(t, PyTuple_GetItem(seq, 1));
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

bu_ptr
parse_score(PyObject * seq)
{
    const int n = PyList_Size(seq);
    if (n == 0) throw std::runtime_error("empty score-list");
    PyObject * fl = PyList_GetItem(seq, 0);
    if ( ! PyList_Check(fl))
        throw std::runtime_error("first score-entry must be filter-list");
    score * sc = new score();
    bu_ptr rt = bu_ptr(sc);
    const int k = PyList_Size(fl);
    for (int j=0; j<k; j++) {
        rt = parse_filter(std::move(rt), PyList_GetItem(fl, j));
    }
    for (int i=1; i<n; i++) {
        score_entry && se = parse_entry(PyList_GetItem(seq, i));
        sc->add(se.t * tempo, std::move(se.b));
    }
    return rt;
}

PyObject *
render(PyObject * self, PyObject * args)
{
    PyObject * data;
    if ( ! PyArg_ParseTuple(args, "Od", &data, &tempo))
        return NULL;
    score_entry se(0, data);
    PyObject * o = PyObject_CallObject((PyObject *)&UgenType, NULL);
    reinterpret_cast<UgenObject *>(o)->g = new limiter(se.b->build());
    return o;
}

PyObject *
just(PyObject * self, PyObject * args)
{
    double pitch;
    if ( ! PyArg_ParseTuple(args, "d", &pitch))
        return NULL;
    long lower = pitch;
    double fraction = pitch - lower;
    double f = f_of_just(lower);
    if (fraction > 0.0001) {
        const double g = f_of_just(lower + 1) - f;
        f += fraction * g;
    }
    return PyFloat_FromDouble(f);
}

PyMethodDef methoddef[] = {
    { "render", render, METH_VARARGS, "render." },
    { "just", just, METH_VARARGS, "just." },
    { NULL, NULL, 0, NULL }, /* Sentinel */
};

struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "fuge",
    "doc",
    -1,
    methoddef,
    NULL, NULL, NULL, NULL
};

} //namespace

PyMODINIT_FUNC
PyInit_fuge(void)
{
    UgenType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&UgenType) < 0) return NULL;
    PyObject * m = PyModule_Create(&moduledef);
    if (m == NULL) return NULL;
    Py_INCREF(&UgenType);
    PyModule_AddObject(m, "Ugen", (PyObject *)&UgenType);
    init_orchestra();
    init_effects();
    return m;
}
